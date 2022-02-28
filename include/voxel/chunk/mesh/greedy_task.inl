#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/chunk.h"
#include "voxel/chunk/grid.h"

template <ChunkMeshComparator MeshComparator>
void hvox::ChunkGreedyMeshTask<MeshComparator>::execute(ChunkLoadThreadState*, ChunkLoadTaskQueue*) {
    m_chunk->mesh_task_active.store(true, std::memory_order_release);

    // TODO(Matthew): Remove this, but we'll remove it when we shift strategy stuff
    //                to templated policy.
    bool owns_strategy = !m_strategy;
    if (!m_strategy) {
        m_strategy = reinterpret_cast<void*>(
            new ChunkMeshStrategy(
                [&](const Block* source, const Block* target, BlockChunkPosition, Chunk*) {
                    return (source->id == target->id) && (source->id != 0);
                }
            )
        );
    }

    // TODO(Matthew): Cross-chunk greedy instancing?
    // Note that this code would probably need to be enabled if we were to do
    // greedy instancing across chunk boundaries, which for now we do not do.
    //   Admittedly such a strategy would require more substantial changes anyway,
    //   in particular instructing neighbouring chunks that they will also be
    //   due a remeshing.
    //     The benefits in reduced instances would likely be outweighed by this cost.
    // // Only execute if all preloaded neighbouring chunks have at least been generated.
    // auto [ _, neighbours_in_required_state ] =
    //         m_chunk_grid->query_all_neighbour_states(m_chunk, ChunkState::GENERATED);

    // if (!neighbours_in_required_state) {
    //     // Mark as no longer engaging in this meshing task.
    //     m_chunk->mesh_task_active.store(false, std::memory_order_release);
    //     // Put copy of this mesh task back onto the load task queue.
    //     ChunkGreedyMeshTask<MeshComparator>* mesh_task = new ChunkGreedyMeshTask<MeshComparator>();
    //     mesh_task->init(m_chunk, m_chunk_grid, nullptr);
    //     task_queue->enqueue(state->producer_token, { mesh_task, true });
    //     m_chunk->pending_task.store(ChunkLoadTaskKind::MESH, std::memory_order_release);
    //     return;
    // }

    // TODO(Matthew): Better guess work should be possible and expand only when needed.
    //                  Maybe in addition to managing how all chunk's transformations are
    //                  stored on GPU, ChunkGrid-level should also manage this data?
    //                    This could get hard with scalings as well.
    // TODO(Matthew):       For greedy meshing, while translations will by definition be
    //                      unique, scalings will not be, and so an index buffer could
    //                      further improve performance and also remove the difficulty
    //                      of the above TODO.
    m_chunk->instance = {
        new ChunkInstanceData[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE],
        0
    };

    Block* blocks = m_chunk->blocks;

    std::queue<BlockChunkPosition> queued_for_visit;

    bool* visited = new bool[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE]{false};

    auto  data        = m_chunk->instance.data;
    auto& voxel_count = m_chunk->instance.count;

    const Block*        source = &blocks[0];
    BlockChunkPosition  start  = BlockChunkPosition{0};
    BlockChunkPosition  end    = BlockChunkPosition{0};
    BlockChunkPosition  target_pos;

    // Determines if two blocks are of the same mesheable kind.
    const MeshComparator are_same_instanceable{};

    bool blocks_to_consider = true;
    while (blocks_to_consider) {
process_new_source:
        bool found_instanceable = are_same_instanceable(source, source, {}, m_chunk);

        /***************\
         * Scan X - 1D *
        \***************/

        target_pos = start;
        for (; target_pos.x < CHUNK_SIZE; ++target_pos.x) {
            auto target_idx = block_index(target_pos);

            const Block* target = &blocks[target_idx];
            // We are scanning for a new instanceable source block.
            if (!found_instanceable) {
                // Found a instanceable source block that hasn't already
                // been visited.
                if (are_same_instanceable(target, target, target_pos, m_chunk)
                        && !visited[target_idx]) {
                    source = target;
                    start  = target_pos;
                    end    = target_pos;

                    goto process_new_source;
                // Current block we're looking at is either already visited or
                // not an instanceable source block. Set it as visited.
                } else {
                    visited[target_idx] = true;
                    continue;
                }
            // We are scanning for the extent of an instanceable source block.
            } else {
                if (!are_same_instanceable(source, target, target_pos, m_chunk)
                        || visited[target_idx]) {
                    end.x = target_pos.x - 1;

                    break;
                }
            }
        }

        // If we scanned all the way, we won't have set end component, so set it.
        if (target_pos.x == CHUNK_SIZE)
            end.x = CHUNK_SIZE - 1;

        // If we were scanning for the extent of an instanceable source block,
        // we now set the visited status of scanned blocks.
        if (found_instanceable) {
            set_per_block_data(
                visited,
                start,
                end,
                true
            );
        }

        /***************\
         * Scan Z - 2D *
        \***************/

        target_pos = start + BlockChunkPosition{0, 0, 1};
        for (; target_pos.z < CHUNK_SIZE; ++target_pos.z) {
            bool done = false;

            // Scan across X for each Z coord. Should any given Z's X-wise scan
            // not permit an extent the same length in X as the first scanned,
            // we reject that Z coord from being in the current extent and start
            // the next scan.
            for (target_pos.x = start.x; target_pos.x <= end.x; ++target_pos.x) {
                auto target_idx = block_index(target_pos);

                const Block* target = &blocks[target_idx];
                // We are scanning for a new instanceable source block.
                if (!found_instanceable) {
                    // Found a instanceable source block that hasn't already
                    // been visited.
                    if (are_same_instanceable(target, target, target_pos, m_chunk)
                            && !visited[target_idx]) {
                        source = target;
                        start  = target_pos;
                        end    = target_pos;

                        goto process_new_source;
                    // Current block we're looking at is either already visited or
                    // not an instanceable source block. Set it as visited.
                    } else {
                        visited[target_idx] = true;
                        continue;
                    }
                // We are scanning for the extent of an instanceable source block.
                } else if (!are_same_instanceable(source, target, target_pos, m_chunk)
                                || visited[target_idx]) {
                    end.z = target_pos.z - 1;

                    done = true;
                    break;
                }
            }

            if (done) break;
        }

        // If we scanned all the way, we won't have set end component, so set it.
        if (target_pos.z == CHUNK_SIZE)
            end.z = CHUNK_SIZE - 1;

        // If we were scanning for the extent of an instanceable source block,
        // we now set the visited status of scanned blocks.
        if (found_instanceable) {
            set_per_block_data(
                visited,
                start + BlockChunkPosition{0, 0, 1},
                end,
                true
            );
        }

        /***************\
         * Scan Y - 3D *
        \***************/

        target_pos = start + BlockChunkPosition{0, 1, 0};
        for (; target_pos.y < CHUNK_SIZE; ++target_pos.y) {
            bool done = false;

            // Scan across X-Z plane for each Y coord. Should any given Y's
            // XZ-wise scan not permit an extent the same length in X as the
            // first scanned, we reject that X-Z plane from being in the
            // current extent and start the next scan.
            for (target_pos.z = start.z; target_pos.z <= end.z; ++target_pos.z) {
                for (target_pos.x = start.x; target_pos.x <= end.x; ++target_pos.x) {
                    auto target_idx = block_index(target_pos);

                    const Block* target = &blocks[target_idx];
                    // We are scanning for a new instanceable source block.
                    if (!found_instanceable) {
                        // Found a instanceable source block that hasn't already
                        // been visited.
                        if (are_same_instanceable(target, target, target_pos, m_chunk)
                                && !visited[target_idx]) {
                            source = target;
                            start  = target_pos;
                            end    = target_pos;

                            goto process_new_source;
                        // Current block we're looking at is either already visited or
                        // not an instanceable source block. Set it as visited.
                        } else {
                            visited[target_idx] = true;
                            continue;
                        }
                    // We are scanning for the extent of an instanceable source block.
                    } else if (!are_same_instanceable(source, target, target_pos, m_chunk)
                                    || visited[target_idx]) {
                        end.y = target_pos.y - 1;

                        done = true;
                        break;
                    }
                }

                if (done) break;
            }

            if (done) break;
        }

        // If we scanned all the way, we won't have set end component, so set it.
        if (target_pos.y == CHUNK_SIZE)
            end.y = CHUNK_SIZE - 1;

        // Add blocks adjacent to X-face to queue.
        if (end.x != CHUNK_SIZE - 1) {
            for (ui32 z = start.z; z <= end.z; ++z) {
                for (ui32 y = start.y; y <= end.y; ++y) {
                    queued_for_visit.push({end.x + 1, y, z});
                }
            }
        }
        // Add blocks adjacent to Z-face to queue.
        if (end.z != CHUNK_SIZE - 1) {
            for (ui32 x = start.x; x <= end.x; ++x) {
                for (ui32 y = start.y; y <= end.y; ++y) {
                    queued_for_visit.push({x, y, end.z + 1});
                }
            }
        }
        // Add blocks adjacent to Y-face to queue.
        if (end.y != CHUNK_SIZE - 1) {
            for (ui32 x = start.x; x <= end.x; ++x) {
                for (ui32 z = start.z; z <= end.z; ++z) {
                    queued_for_visit.push({x, end.y + 1, z});
                }
            }
        }

        // If we were scanning for the extent of an instanceable source block,
        // we now set the visited status of scanned blocks.
        if (found_instanceable) {
            set_per_block_data(
                visited,
                start + BlockChunkPosition{0, 1, 0},
                end,
                true
            );
        }

        /*******************\
         * Create Instance *
        \*******************/

        if (found_instanceable) {
            BlockWorldPosition start_world = block_world_position(m_chunk->position, start);
            BlockWorldPosition end_world   = block_world_position(m_chunk->position, end);

            f32v3 centre_of_cuboid = (f32v3{end_world} - f32v3{start_world}) / 2.0f;
            f32v3 centre_of_cuboid_in_world = centre_of_cuboid + f32v3{start_world};
            f32v3 scale_of_cuboid  =  f32v3{end_world} - f32v3{start_world} + f32v3{1.0f};

            data[voxel_count++] = ChunkInstanceData{ centre_of_cuboid_in_world, scale_of_cuboid };
        }

        /***************\
         * Pump Queue  *
        \***************/

        // If we have got no more blocks to visit, then we are done.
        if (queued_for_visit.empty())
            break;

        // Pump queue for first block that we have not yet visited.
        do {
            start  = queued_for_visit.front();
            end    = start;
            source = &blocks[block_index(start)];

            queued_for_visit.pop();

            // If we've found a block we have yet to visit, process
            // it next.
            if (!visited[block_index(start)])
                break;

            // If we have emtpied the queue without finding a block
            // we have yet to visit, then we are done.
            if (queued_for_visit.empty()) {
                blocks_to_consider = false;
                break;
            }
        } while (true);
    };

    delete[] visited;
    if (owns_strategy)
        delete reinterpret_cast<ChunkMeshStrategy*>(m_strategy);

    m_chunk->mesh_task_active.store(false, std::memory_order_release);

    m_chunk->state.store(ChunkState::MESHED, std::memory_order_release);
}

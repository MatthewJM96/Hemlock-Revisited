#include "stdafx.h"

#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/chunk.h"
#include "voxel/grid.h"

#include "voxel/mesher.h"

static inline bool is_at_left_face(hvox::BlockIndex index) {
    return (index % CHUNK_SIZE) == 0;
}
static inline bool is_at_right_face(hvox::BlockIndex index) {
    return ((index + 1) % CHUNK_SIZE) == 0;
}
static inline bool is_at_bottom_face(hvox::BlockIndex index) {
    return (index % (CHUNK_SIZE * CHUNK_SIZE)) < CHUNK_SIZE;
}
static inline bool is_at_top_face(hvox::BlockIndex index) {
    return (index % (CHUNK_SIZE * CHUNK_SIZE)) >= (CHUNK_SIZE * (CHUNK_SIZE - 1));
}
static inline bool is_at_front_face(hvox::BlockIndex index) {
    return index < (CHUNK_SIZE * CHUNK_SIZE);
}
static inline bool is_at_back_face(hvox::BlockIndex index) {
    return index >= (CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1));
}

static inline hvox::BlockIndex index_at_right_face(hvox::BlockIndex index) {
    return index + CHUNK_SIZE - 1;
}
static inline hvox::BlockIndex index_at_left_face(hvox::BlockIndex index) {
    return index - CHUNK_SIZE + 1;
}
static inline hvox::BlockIndex index_at_top_face(hvox::BlockIndex index) {
    return index + (CHUNK_SIZE * (CHUNK_SIZE - 1));
}
static inline hvox::BlockIndex index_at_bottom_face(hvox::BlockIndex index) {
    return index - (CHUNK_SIZE * (CHUNK_SIZE - 1));
}
static inline hvox::BlockIndex index_at_front_face(hvox::BlockIndex index) {
    return index - (CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1));
}
static inline hvox::BlockIndex index_at_back_face(hvox::BlockIndex index) {
    return index + (CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1));
}

void hvox::ChunkMeshTask::execute(ChunkLoadThreadState*, ChunkLoadTaskQueue*) {
    // TODO(Matthew): Cross-chunk greedy instancing?
    // Note that this code would probably need to be reenabled if we were to do
    // greedy instancing across chunk boundaries, which for now we do not do.
    //   Admittedly such a change would require more substantial changes anyway.
    // // Only execute if all preloaded neighbouring chunks have at least been generated.
    // auto [ _, neighbours_in_required_state ] =
    //         m_chunk_grid->query_all_neighbour_states(m_chunk, ChunkState::GENERATED);

    // if (!neighbours_in_required_state) {
    //     // Put this mesh task back onto the load queue.
    //     ChunkMeshTask* mesh_task = new ChunkMeshTask();
    //     mesh_task->init(m_chunk, m_chunk_grid);
    //     task_queue->enqueue(state->producer_token, { mesh_task, true });
    //     m_chunk->pending_task.store(ChunkLoadTaskKind::MESH, std::memory_order_release);
    //     return;
    // }

    m_chunk->instance = { nullptr, 0 };

    Block* blocks = m_chunk->blocks;

    std::queue<BlockChunkPosition> queued_for_visit;

    bool* visited = new bool[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE]{false};

    // TODO(Matthew): Better guess work should be possible and expand only when needed.
    //                  Maybe in addition to managing how all chunk's transformations are
    //                  stored on GPU, ChunkGrid-level should also manage this data?
    //                    This could get hard with scalings as well.
    // TODO(Matthew):       For greedy meshing, while translations will by definition be
    //                      unique, scalings will not be, and so an index buffer could
    //                      further improve performance and also remove the difficulty
    //                      of the above TODO.
    m_chunk->instance.data = new ChunkInstanceData[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 2];

    auto  data        = m_chunk->instance.data;
    auto& voxel_count = m_chunk->instance.count;

    // TODO(Matthew): In empty block check, do we want to break out of a given
    //                set of loops entirely if we encounter a visited block?
    //                  Unsure if this could actually happen in practice.
    // TODO(Matthew): Assuming block ID is sufficient to differentiate blocks.
    //                  We can't generally expect that.
    BlockID             kind   = blocks[0].id;
    BlockChunkPosition  start  = BlockChunkPosition{0};
    BlockChunkPosition  end    = BlockChunkPosition{0};
    do {
        if (!queued_for_visit.empty()) {
            queued_for_visit.pop();
        }
start_loop:
        BlockChunkPosition candidate = start;
        bool have_found_instanceable = true;
        for (; candidate.x < CHUNK_SIZE; ++candidate.x) {
            if (kind == 0) {
                if (blocks[block_index(candidate)].id != 0
                        && !visited[block_index(candidate)]) {
                    start = candidate;
                    end   = start;
                    kind  = blocks[block_index(candidate)].id;

                    have_found_instanceable = true;
                    goto start_loop;
                } else {
                    have_found_instanceable = false;
                    visited[block_index(candidate)] = true;
                    continue;
                }
            }

            if (kind != blocks[block_index(candidate)].id
                    || visited[block_index(candidate)]) {
                end = { candidate.x - 1, 0, 0 };

                queued_for_visit.push({candidate.x, start.y, start.z});

                break;
            }

            visited[block_index(candidate)] = true;
        }

        if (candidate.x == CHUNK_SIZE)
            end = { candidate.x - 1, 0, 0 };

        candidate = start + BlockChunkPosition{0, 0, 1};
        for (; candidate.z < CHUNK_SIZE; ++candidate.z) {
            bool done = false;

            for (candidate.x = start.x; candidate.x <= end.x; ++candidate.x) {
                if (kind == 0) {
                    if (blocks[block_index(candidate)].id != 0
                            && !visited[block_index(candidate)]) {
                        start = candidate;
                        end   = start;
                        kind  = blocks[block_index(candidate)].id;

                        have_found_instanceable = true;
                        goto start_loop;
                    } else {
                        have_found_instanceable = false;
                        visited[block_index(candidate)] = true;
                        continue;
                    }
                }

                if (kind != blocks[block_index(candidate)].id
                        || visited[block_index(candidate)]) {
                    end = { end.x, 0, candidate.z - 1};

                    queued_for_visit.push({start.x, start.y, candidate.z});

                    done = true;
                    break;
                }

                visited[block_index(candidate)] = true;
            }

            if (done) break;
        }

        if (candidate.z == CHUNK_SIZE)
            end = { end.x, 0, candidate.z - 1};

        candidate = start + BlockChunkPosition{0, 1, 0};
        for (; candidate.y < CHUNK_SIZE; ++candidate.y) {
            bool done = false;

            for (candidate.z = start.z; candidate.z <= end.z; ++candidate.z) {
                for (candidate.x = start.x; candidate.x <= end.x; ++candidate.x) {
                    if (kind == 0) {
                        if (blocks[block_index(candidate)].id != 0
                                && !visited[block_index(candidate)]) {
                            start = candidate;
                            end   = start;
                            kind  = blocks[block_index(candidate)].id;

                            have_found_instanceable = true;
                            goto start_loop;
                        } else {
                            have_found_instanceable = false;
                            visited[block_index(candidate)] = true;
                            continue;
                        }
                    }

                    if (kind != blocks[block_index(candidate)].id
                            || visited[block_index(candidate)]) {
                        end = { end.x, candidate.y - 1, end.z};

                        queued_for_visit.push({start.x, candidate.y, start.z});

                        done = true;
                        break;
                    }

                    visited[block_index(candidate)] = true;
                }

                if (done) break;
            }

            if (done) break;
        }

        if (candidate.y == CHUNK_SIZE)
            end = { end.x, candidate.y - 1, end.z};

        if (have_found_instanceable) {
            BlockWorldPosition start_world = block_world_position(m_chunk->position, start);
            BlockWorldPosition end_world   = block_world_position(m_chunk->position, end);

            f32v3 centre_of_cuboid = (f32v3{end_world} - f32v3{start_world}) / 2.0f;
            f32v3 centre_of_cuboid_in_world = centre_of_cuboid + f32v3{start_world};
            f32v3 scale_of_cuboid  =  f32v3{end_world} - f32v3{start_world} + f32v3{1.0f};

            data[voxel_count++] = ChunkInstanceData{ centre_of_cuboid_in_world, scale_of_cuboid };
        }
pump_queue:
        if (queued_for_visit.empty())
            break;

        start  = queued_for_visit.front();
        end    = start;
        kind   = blocks[block_index(start)].id;

        if (visited[block_index(start)]) {
            queued_for_visit.pop();
            goto pump_queue;
        }
    } while (!queued_for_visit.empty());

    delete[] visited;

    m_chunk->state.store(ChunkState::MESHED, std::memory_order_release);
}

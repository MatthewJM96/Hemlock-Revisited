#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/chunk/chunk.h"
#include "voxel/chunk/grid.h"
#include "voxel/chunk/setter.hpp"

template <hvox::IdealBlockComparator MeshComparator>
bool hvox::GreedyMeshStrategy<
    MeshComparator>::can_run(hmem::Handle<ChunkGrid>, hmem::Handle<Chunk>) const {
    return true;
}

template <hvox::IdealBlockComparator MeshComparator>
void hvox::GreedyMeshStrategy<MeshComparator>::operator()(
    hmem::Handle<ChunkGrid>, hmem::Handle<Chunk> chunk
) const {
    // TODO(Matthew): Better guess work should be possible and expand only when
    // needed.
    //                  Maybe in addition to managing how all chunk's transformations
    //                  are stored on GPU, ChunkGrid-level should also manage this
    //                  data?
    //                    This could get hard with scalings as well.
    // TODO(Matthew):       For greedy meshing, while translations will by definition
    // be
    //                      unique, scalings will not be, and so an index buffer could
    //                      further improve performance and also remove the difficulty
    //                      of the above TODO.

    std::shared_lock<std::shared_mutex> block_lock;
    auto                                blocks = chunk->blocks.get(block_lock);

    std::queue<BlockChunkPosition> queued_for_visit;

    bool* visited = new bool[CHUNK_VOLUME]{ false };

    chunk->instance.generate_buffer();

    std::unique_lock<std::shared_mutex> instance_lock;
    auto&                               instance = chunk->instance.get(instance_lock);

    const Block*       source = &blocks[0];
    BlockChunkPosition start  = BlockChunkPosition{ 0 };
    BlockChunkPosition end    = BlockChunkPosition{ 0 };
    BlockChunkPosition target_pos;

    // Determines if two blocks are of the same mesheable kind.
    const MeshComparator are_same_instanceable{};

    auto add_border_blocks_to_queue
        = [&](BlockChunkPosition _start, BlockChunkPosition _end) {
              // Add blocks adjacent to X-face to queue.
              if (_end.x != CHUNK_LENGTH - 1) {
                  for (ui32 z = _start.z; z <= _end.z; ++z) {
                      for (ui32 y = _start.y; y <= _end.y; ++y) {
                          queued_for_visit.push({ _end.x + 1, y, z });
                      }
                  }
              }
              // Add blocks adjacent to Z-face to queue.
              if (_end.z != CHUNK_LENGTH - 1) {
                  for (ui32 x = _start.x; x <= _end.x; ++x) {
                      for (ui32 y = _start.y; y <= _end.y; ++y) {
                          queued_for_visit.push({ x, y, _end.z + 1 });
                      }
                  }
              }
              // Add blocks adjacent to Y-face to queue.
              if (_end.y != CHUNK_LENGTH - 1) {
                  for (ui32 x = _start.x; x <= _end.x; ++x) {
                      for (ui32 z = _start.z; z <= _end.z; ++z) {
                          queued_for_visit.push({ x, _end.y + 1, z });
                      }
                  }
              }
          };

    Chunk* raw_chunk_ptr = chunk.get();

    bool blocks_to_consider = true;
    while (blocks_to_consider) {
process_new_source:
        bool found_instanceable
            = are_same_instanceable(source, source, {}, raw_chunk_ptr);

        /***************\
         * Scan X - 1D *
        \***************/

        target_pos = start;
        for (; target_pos.x < CHUNK_LENGTH; ++target_pos.x) {
            auto target_idx = block_index(target_pos);

            const Block* target = &blocks[target_idx];
            // We are scanning for a new instanceable source block.
            if (!found_instanceable) {
                // Found a instanceable source block that hasn't already
                // been visited.
                if (are_same_instanceable(target, target, target_pos, raw_chunk_ptr)
                    && !visited[target_idx])
                {
                    add_border_blocks_to_queue(start, target_pos);

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
                if (!are_same_instanceable(source, target, target_pos, raw_chunk_ptr)
                    || visited[target_idx])
                {
                    end.x = target_pos.x - 1;

                    break;
                }
            }
        }

        // If we scanned all the way, we won't have set end component, so set it.
        if (target_pos.x == CHUNK_LENGTH) end.x = CHUNK_LENGTH - 1;

        // If we were scanning for the extent of an instanceable source block,
        // we now set the visited status of scanned blocks.
        if (found_instanceable) {
            set_per_block_data(visited, start, end, true);
        }

        /***************\
         * Scan Z - 2D *
        \***************/

        target_pos = start + BlockChunkPosition{ 0, 0, 1 };
        for (; target_pos.z < CHUNK_LENGTH; ++target_pos.z) {
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
                    if (are_same_instanceable(target, target, target_pos, raw_chunk_ptr)
                        && !visited[target_idx])
                    {
                        add_border_blocks_to_queue(start, target_pos);

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
                } else if (!are_same_instanceable(source, target, target_pos, raw_chunk_ptr) || visited[target_idx])
                {
                    end.z = target_pos.z - 1;

                    done = true;
                    break;
                }
            }

            if (done) break;
        }

        // If we scanned all the way, we won't have set end component, so set it.
        if (target_pos.z == CHUNK_LENGTH) end.z = CHUNK_LENGTH - 1;

        // If we were scanning for the extent of an instanceable source block,
        // we now set the visited status of scanned blocks.
        if (found_instanceable) {
            set_per_block_data(
                visited, start + BlockChunkPosition{ 0, 0, 1 }, end, true
            );
        }

        /***************\
         * Scan Y - 3D *
        \***************/

        target_pos = start + BlockChunkPosition{ 0, 1, 0 };
        for (; target_pos.y < CHUNK_LENGTH; ++target_pos.y) {
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
                        if (are_same_instanceable(
                                target, target, target_pos, raw_chunk_ptr
                            )
                            && !visited[target_idx])
                        {
                            add_border_blocks_to_queue(start, target_pos);

                            source = target;
                            start  = target_pos;
                            end    = target_pos;

                            goto process_new_source;
                            // Current block we're looking at is either already
                            // visited or not an instanceable source block. Set it as
                            // visited.
                        } else {
                            visited[target_idx] = true;
                            continue;
                        }
                        // We are scanning for the extent of an instanceable source
                        // block.
                    } else if (!are_same_instanceable(source, target, target_pos, raw_chunk_ptr) || visited[target_idx])
                    {
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
        if (target_pos.y == CHUNK_LENGTH) end.y = CHUNK_LENGTH - 1;

        add_border_blocks_to_queue(start, end);

        // If we were scanning for the extent of an instanceable source block,
        // we now set the visited status of scanned blocks.
        if (found_instanceable) {
            set_per_block_data(
                visited, start + BlockChunkPosition{ 0, 1, 0 }, end, true
            );
        }

        /*******************\
         * Create Instance *
        \*******************/

        if (found_instanceable) {
            BlockWorldPosition start_instance
                = block_world_position(chunk->position, start);
            BlockWorldPosition end_instance
                = block_world_position(chunk->position, end);

            f32v3 scale_of_cuboid
                = f32v3{ end_instance } - f32v3{ start_instance } + f32v3{ 1.0f };

            instance.data[instance.count++]
                = ChunkInstanceData{ start_instance, scale_of_cuboid };
        }

        /***************\
         * Pump Queue  *
        \***************/

        // If we have got no more blocks to visit, then we are done.
        if (queued_for_visit.empty()) break;

        // Pump queue for first block that we have not yet visited.
        do {
            start  = queued_for_visit.front();
            end    = start;
            source = &blocks[block_index(start)];

            queued_for_visit.pop();

            // If we've found a block we have yet to visit, process
            // it next.
            if (!visited[block_index(start)]) break;

            // If we have emtpied the queue without finding a block
            // we have yet to visit, then we are done.
            if (queued_for_visit.empty()) {
                blocks_to_consider = false;
                break;
            }
        } while (true);
    };

    delete[] visited;
}

#include "graphics/mesh.h"
#include "voxel/chunk/chunk.h"
#include "voxel/chunk/grid.h"
#include "voxel/chunk/setter.hpp"
#include "voxel/state.hpp"

template <hvox::IdealVoxelComparator MeshComparator>
bool hvox::GreedyMeshStrategy<
    MeshComparator>::can_run(hmem::Handle<ChunkGrid>, hmem::Handle<Chunk>) const {
    return true;
}

template <hvox::IdealVoxelComparator MeshComparator>
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

    std::shared_lock<std::shared_mutex> voxel_lock;
    auto                                voxels = chunk->voxels.get(voxel_lock);

    std::queue<VoxelChunkPosition> queued_for_visit;

    bool* visited = new bool[CHUNK_VOLUME]{ false };

    chunk->instance.generate_buffer();

    std::unique_lock<std::shared_mutex> mesh_lock;
    auto&                               mesh = chunk->instance.get(mesh_lock);

    Voxel              source = voxels[0];
    VoxelChunkPosition start  = VoxelChunkPosition{ 0 };
    VoxelChunkPosition end    = VoxelChunkPosition{ 0 };
    VoxelChunkPosition target_pos;

    // Determines if two voxels are of the same mesheable kind.
    const MeshComparator are_same_meshable{};

    auto add_border_voxels_to_queue
        = [&](VoxelChunkPosition _start, VoxelChunkPosition _end) {
              // Add voxels adjacent to X-face to queue.
              if (_end.x != CHUNK_LENGTH - 1) {
                  for (ui32 z = _start.z; z <= _end.z; ++z) {
                      for (ui32 y = _start.y; y <= _end.y; ++y) {
                          queued_for_visit.push({ _end.x + 1, y, z });
                      }
                  }
              }
              // Add voxels adjacent to Z-face to queue.
              if (_end.z != CHUNK_LENGTH - 1) {
                  for (ui32 x = _start.x; x <= _end.x; ++x) {
                      for (ui32 y = _start.y; y <= _end.y; ++y) {
                          queued_for_visit.push({ x, y, _end.z + 1 });
                      }
                  }
              }
              // Add voxels adjacent to Y-face to queue.
              if (_end.y != CHUNK_LENGTH - 1) {
                  for (ui32 x = _start.x; x <= _end.x; ++x) {
                      for (ui32 z = _start.z; z <= _end.z; ++z) {
                          queued_for_visit.push({ x, _end.y + 1, z });
                      }
                  }
              }
          };

    Chunk* raw_chunk_ptr = chunk.get();

    bool voxels_to_consider = true;
    while (voxels_to_consider) {
process_new_source:
        bool found_meshable = are_same_meshable(source, source, {}, raw_chunk_ptr);

        /***************\
         * Scan X - 1D *
        \***************/

        target_pos = start;
        for (; target_pos.x < CHUNK_LENGTH; ++target_pos.x) {
            auto target_idx = voxel_index(target_pos);

            Voxel target = voxels[target_idx];
            // We are scanning for a new meshable source voxel.
            if (!found_meshable) {
                // Found a meshable source voxel that hasn't already
                // been visited.
                if (are_same_meshable(target, target, target_pos, raw_chunk_ptr)
                    && !visited[target_idx])
                {
                    add_border_voxels_to_queue(start, target_pos);

                    source = target;
                    start  = target_pos;
                    end    = target_pos;

                    goto process_new_source;
                    // Current voxel we're looking at is either already visited or
                    // not an meshable source voxel. Set it as visited.
                } else {
                    visited[target_idx] = true;
                    continue;
                }
                // We are scanning for the extent of an meshable source voxel.
            } else {
                if (!are_same_meshable(source, target, target_pos, raw_chunk_ptr)
                    || visited[target_idx])
                {
                    end.x = target_pos.x - 1;

                    break;
                }
            }
        }

        // If we scanned all the way, we won't have set end component, so set it.
        if (target_pos.x == CHUNK_LENGTH) end.x = CHUNK_LENGTH - 1;

        // If we were scanning for the extent of an meshable source voxel,
        // we now set the visited status of scanned voxels.
        if (found_meshable) {
            set_per_voxel_data(visited, start, end, true);
        }

        /***************\
         * Scan Z - 2D *
        \***************/

        target_pos = start + VoxelChunkPosition{ 0, 0, 1 };
        for (; target_pos.z < CHUNK_LENGTH; ++target_pos.z) {
            bool done = false;

            // Scan across X for each Z coord. Should any given Z's X-wise scan
            // not permit an extent the same length in X as the first scanned,
            // we reject that Z coord from being in the current extent and start
            // the next scan.
            for (target_pos.x = start.x; target_pos.x <= end.x; ++target_pos.x) {
                auto target_idx = voxel_index(target_pos);

                Voxel target = voxels[target_idx];
                // We are scanning for a new meshable source voxel.
                if (!found_meshable) {
                    // Found a meshable source voxel that hasn't already
                    // been visited.
                    if (are_same_meshable(target, target, target_pos, raw_chunk_ptr)
                        && !visited[target_idx])
                    {
                        add_border_voxels_to_queue(start, target_pos);

                        source = target;
                        start  = target_pos;
                        end    = target_pos;

                        goto process_new_source;
                        // Current voxel we're looking at is either already visited or
                        // not an meshable source voxel. Set it as visited.
                    } else {
                        visited[target_idx] = true;
                        continue;
                    }
                    // We are scanning for the extent of an meshable source voxel.
                } else if (!are_same_meshable(source, target, target_pos, raw_chunk_ptr) || visited[target_idx])
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

        // If we were scanning for the extent of an meshable source voxel,
        // we now set the visited status of scanned voxels.
        if (found_meshable) {
            set_per_voxel_data(
                visited, start + VoxelChunkPosition{ 0, 0, 1 }, end, true
            );
        }

        /***************\
         * Scan Y - 3D *
        \***************/

        target_pos = start + VoxelChunkPosition{ 0, 1, 0 };
        for (; target_pos.y < CHUNK_LENGTH; ++target_pos.y) {
            bool done = false;

            // Scan across X-Z plane for each Y coord. Should any given Y's
            // XZ-wise scan not permit an extent the same length in X as the
            // first scanned, we reject that X-Z plane from being in the
            // current extent and start the next scan.
            for (target_pos.z = start.z; target_pos.z <= end.z; ++target_pos.z) {
                for (target_pos.x = start.x; target_pos.x <= end.x; ++target_pos.x) {
                    auto target_idx = voxel_index(target_pos);

                    Voxel target = voxels[target_idx];
                    // We are scanning for a new meshable source voxel.
                    if (!found_meshable) {
                        // Found a meshable source voxel that hasn't already
                        // been visited.
                        if (are_same_meshable(target, target, target_pos, raw_chunk_ptr)
                            && !visited[target_idx])
                        {
                            add_border_voxels_to_queue(start, target_pos);

                            source = target;
                            start  = target_pos;
                            end    = target_pos;

                            goto process_new_source;
                            // Current voxel we're looking at is either already
                            // visited or not an meshable source voxel. Set it as
                            // visited.
                        } else {
                            visited[target_idx] = true;
                            continue;
                        }
                        // We are scanning for the extent of an meshable source
                        // voxel.
                    } else if (!are_same_meshable(source, target, target_pos, raw_chunk_ptr) || visited[target_idx])
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

        add_border_voxels_to_queue(start, end);

        // If we were scanning for the extent of an meshable source voxel,
        // we now set the visited status of scanned voxels.
        if (found_meshable) {
            set_per_voxel_data(
                visited, start + VoxelChunkPosition{ 0, 1, 0 }, end, true
            );
        }

        /*******************\
         * Create Instance *
        \*******************/

        if (found_meshable) {
            VoxelWorldPosition start_mesh
                = voxel_world_position(chunk->position, start);
            VoxelWorldPosition end_mesh = voxel_world_position(chunk->position, end);

            f32v3 scale_of_cuboid
                = f32v3{ end_mesh } - f32v3{ start_mesh } + f32v3{ 1.0f };

            mesh.data[mesh.count++] = ChunkInstanceData{ start_mesh, scale_of_cuboid };
        }

        /***************\
         * Pump Queue  *
        \***************/

        // If we have got no more voxels to visit, then we are done.
        if (queued_for_visit.empty()) break;

        // Pump queue for first voxel that we have not yet visited.
        do {
            start  = queued_for_visit.front();
            end    = start;
            source = voxels[voxel_index(start)];

            queued_for_visit.pop();

            // If we've found a voxel we have yet to visit, process
            // it next.
            if (!visited[voxel_index(start)]) break;

            // If we have emtpied the queue without finding a voxel
            // we have yet to visit, then we are done.
            if (queued_for_visit.empty()) {
                voxels_to_consider = false;
                break;
            }
        } while (true);
    };

    delete[] visited;
}

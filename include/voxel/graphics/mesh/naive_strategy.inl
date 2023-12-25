#include "graphics/mesh.h"
#include "voxel/chunk/grid.h"
#include "voxel/face_check.hpp"
#include "voxel/voxel.hpp"

template <hvox::IdealVoxelComparator MeshComparator>
bool hvox::NaiveMeshStrategy<
    MeshComparator>::can_run(hmem::Handle<ChunkGrid>, hmem::Handle<Chunk>) const {
    // Only execute if all preloaded neighbouring chunks have at least been
    // generated.
    // TODO(Matthew): Revisit this. query_all_neighbour_states has been removed, do we
    //                stand by this? Further, do we stand by the condition here? Perhaps
    //                we don't really care about the saved vertices.
    // auto [_, neighbours_in_required_state]
    //     = chunk_grid->query_all_neighbour_states(chunk, ChunkState::GENERATED);

    // return neighbours_in_required_state;
    return false;
}

template <hvox::IdealVoxelComparator MeshComparator>
void hvox::NaiveMeshStrategy<MeshComparator>::operator()(
    hmem::Handle<ChunkGrid>, hmem::Handle<Chunk> chunk
) const {
    // TODO(Matthew): Better guess work should be possible and expand only when
    // needed.
    //                  Maybe in addition to managing how all chunk's transformations
    //                  are stored on GPU, ChunkGrid-level should also manage this
    //                  data?
    //                    This could get hard with scalings as well (as will come from
    //                    something like a greedy "meshing" algorithm).
    // TODO(Matthew):       For greedy meshing, while translations will by definition
    // be
    //                      unique, scalings will not be, and so an index buffer could
    //                      further improve performance and also remove the difficulty
    //                      of the above TODO.

    chunk->instance.generate_buffer();

    std::unique_lock<std::shared_mutex> mesh_lock;
    auto&                               mesh = chunk->instance.get(mesh_lock);

    // Determines if voxel is meshable.
    const MeshComparator meshable{};

    auto add_voxel = [&](VoxelWorldPosition pos) {
        mesh.data[mesh.count++] = { f32v3(pos), f32v3(1.0f) };
    };

    Chunk* raw_chunk_ptr = chunk.get();

    std::shared_lock<std::shared_mutex> voxel_lock;
    auto                                voxels = chunk->voxels.get(voxel_lock);

    std::shared_lock<std::shared_mutex> neighbour_lock;

    // TODO(Matthew): Checking voxel is NULL_VOXEL is wrong check really, we will have
    // transparent voxels
    //                e.g. air, to account for too.
    for (VoxelIndex i = 0; i < CHUNK_VOLUME; ++i) {
        const Voxel& voxel = voxels[i];
        if (voxel != NULL_VOXEL) {
            VoxelWorldPosition voxel_position
                = voxel_world_position(chunk->position, i);

            hmem::Handle<Chunk> neighbour;

            // Check its neighbours, to decide whether to add its quads.
            // LEFT
            if (is_at_left_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                VoxelIndex j = index_at_right_face(i);
                neighbour    = chunk->neighbours.one.left.lock();
                if (neighbour) {
                    auto neighbour_voxels = neighbour->voxels.get(neighbour_lock);
                    if (neighbour_voxels[j] == NULL_VOXEL) {
                        add_voxel(voxel_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &voxels[i - 1],
                        &voxels[i - 1],
                        voxel_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_voxel(voxel_position);
                    continue;
                }
            }

            // RIGHT
            if (is_at_right_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                VoxelIndex j = index_at_left_face(i);
                neighbour    = chunk->neighbours.one.right.lock();
                if (neighbour) {
                    auto neighbour_voxels = neighbour->voxels.get(neighbour_lock);
                    if (neighbour_voxels[j] == NULL_VOXEL) {
                        add_voxel(voxel_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &voxels[i + 1],
                        &voxels[i + 1],
                        voxel_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_voxel(voxel_position);
                    continue;
                }
            }

            // BOTTOM
            if (is_at_bottom_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                VoxelIndex j = index_at_top_face(i);
                neighbour    = chunk->neighbours.one.bottom.lock();
                if (neighbour) {
                    auto neighbour_voxels = neighbour->voxels.get(neighbour_lock);
                    if (neighbour_voxels[j] == NULL_VOXEL) {
                        add_voxel(voxel_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &voxels[i - CHUNK_LENGTH],
                        &voxels[i - CHUNK_LENGTH],
                        voxel_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_voxel(voxel_position);
                    continue;
                }
            }

            // TOP
            if (is_at_top_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                VoxelIndex j = index_at_bottom_face(i);
                neighbour    = chunk->neighbours.one.top.lock();
                if (neighbour) {
                    auto neighbour_voxels = neighbour->voxels.get(neighbour_lock);
                    if (neighbour_voxels[j] == NULL_VOXEL) {
                        add_voxel(voxel_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &voxels[i + CHUNK_LENGTH],
                        &voxels[i + CHUNK_LENGTH],
                        voxel_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_voxel(voxel_position);
                    continue;
                }
            }

            // FRONT
            if (is_at_front_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                VoxelIndex j = index_at_back_face(i);
                neighbour    = chunk->neighbours.one.front.lock();
                if (neighbour) {
                    auto neighbour_voxels = neighbour->voxels.get(neighbour_lock);
                    if (neighbour_voxels[j] == NULL_VOXEL) {
                        add_voxel(voxel_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &voxels[i - (CHUNK_AREA)],
                        &voxels[i - (CHUNK_AREA)],
                        voxel_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_voxel(voxel_position);
                    continue;
                }
            }

            // BACK
            if (is_at_back_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                VoxelIndex j = index_at_front_face(i);
                neighbour    = chunk->neighbours.one.back.lock();
                if (neighbour) {
                    auto neighbour_voxels = neighbour->voxels.get(neighbour_lock);
                    if (neighbour_voxels[j] == NULL_VOXEL) {
                        add_voxel(voxel_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &voxels[i + (CHUNK_AREA)],
                        &voxels[i + (CHUNK_AREA)],
                        voxel_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_voxel(voxel_position);
                    continue;
                }
            }
        }
    }

    chunk->meshing.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk->on_mesh_change();
}

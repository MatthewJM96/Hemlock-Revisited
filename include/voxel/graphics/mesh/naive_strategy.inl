#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/chunk/grid.h"
#include "voxel/face_check.hpp"

template <hvox::IdealBlockComparator MeshComparator>
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

template <hvox::IdealBlockComparator MeshComparator>
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

    // Determines if block is meshable.
    const MeshComparator meshable{};

    auto add_block = [&](BlockWorldPosition pos) {
        mesh.data[mesh.count++] = { f32v3(pos), f32v3(1.0f) };
    };

    Chunk* raw_chunk_ptr = chunk.get();

    std::shared_lock<std::shared_mutex> block_lock;
    auto                                blocks = chunk->blocks.get(block_lock);

    std::shared_lock<std::shared_mutex> neighbour_lock;

    // TODO(Matthew): Checking block is NULL_BLOCK is wrong check really, we will have
    // transparent blocks
    //                e.g. air, to account for too.
    for (BlockIndex i = 0; i < CHUNK_VOLUME; ++i) {
        const Block& voxel = blocks[i];
        if (voxel != NULL_BLOCK) {
            BlockWorldPosition block_position
                = block_world_position(chunk->position, i);

            hmem::Handle<Chunk> neighbour;

            // Check its neighbours, to decide whether to add its quads.
            // LEFT
            if (is_at_left_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_right_face(i);
                neighbour    = chunk->neighbours.one.left.lock();
                if (neighbour) {
                    auto neighbour_blocks = neighbour->blocks.get(neighbour_lock);
                    if (neighbour_blocks[j] == NULL_BLOCK) {
                        add_block(block_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &blocks[i - 1],
                        &blocks[i - 1],
                        block_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_block(block_position);
                    continue;
                }
            }

            // RIGHT
            if (is_at_right_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_left_face(i);
                neighbour    = chunk->neighbours.one.right.lock();
                if (neighbour) {
                    auto neighbour_blocks = neighbour->blocks.get(neighbour_lock);
                    if (neighbour_blocks[j] == NULL_BLOCK) {
                        add_block(block_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &blocks[i + 1],
                        &blocks[i + 1],
                        block_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_block(block_position);
                    continue;
                }
            }

            // BOTTOM
            if (is_at_bottom_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_top_face(i);
                neighbour    = chunk->neighbours.one.bottom.lock();
                if (neighbour) {
                    auto neighbour_blocks = neighbour->blocks.get(neighbour_lock);
                    if (neighbour_blocks[j] == NULL_BLOCK) {
                        add_block(block_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &blocks[i - CHUNK_LENGTH],
                        &blocks[i - CHUNK_LENGTH],
                        block_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_block(block_position);
                    continue;
                }
            }

            // TOP
            if (is_at_top_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_bottom_face(i);
                neighbour    = chunk->neighbours.one.top.lock();
                if (neighbour) {
                    auto neighbour_blocks = neighbour->blocks.get(neighbour_lock);
                    if (neighbour_blocks[j] == NULL_BLOCK) {
                        add_block(block_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &blocks[i + CHUNK_LENGTH],
                        &blocks[i + CHUNK_LENGTH],
                        block_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_block(block_position);
                    continue;
                }
            }

            // FRONT
            if (is_at_front_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_back_face(i);
                neighbour    = chunk->neighbours.one.front.lock();
                if (neighbour) {
                    auto neighbour_blocks = neighbour->blocks.get(neighbour_lock);
                    if (neighbour_blocks[j] == NULL_BLOCK) {
                        add_block(block_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &blocks[i - (CHUNK_AREA)],
                        &blocks[i - (CHUNK_AREA)],
                        block_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_block(block_position);
                    continue;
                }
            }

            // BACK
            if (is_at_back_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_front_face(i);
                neighbour    = chunk->neighbours.one.back.lock();
                if (neighbour) {
                    auto neighbour_blocks = neighbour->blocks.get(neighbour_lock);
                    if (neighbour_blocks[j] == NULL_BLOCK) {
                        add_block(block_position);
                        continue;
                    }
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &blocks[i + (CHUNK_AREA)],
                        &blocks[i + (CHUNK_AREA)],
                        block_chunk_position(i),
                        raw_chunk_ptr
                    ))
                {
                    add_block(block_position);
                    continue;
                }
            }
        }
    }

    chunk->meshing.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk->on_mesh_change();
}

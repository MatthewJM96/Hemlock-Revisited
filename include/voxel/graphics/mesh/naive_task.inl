#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/chunk/grid.h"
#include "voxel/face_check.hpp"

template <hvox::IdealBlockComparator MeshComparator>
void hvox::ChunkNaiveMeshTask<MeshComparator>::execute(
    ChunkLoadThreadState* state, ChunkTaskQueue* task_queue
) {
    auto chunk = m_chunk.lock();

    if (chunk == nullptr) return;

    {
        auto chunk_grid = m_chunk_grid.lock();

        if (chunk_grid == nullptr) return;

        // Only execute if all preloaded neighbouring chunks have at least been
        // generated.
        auto [_, neighbours_in_required_state]
            = chunk_grid->query_all_neighbour_states(chunk, ChunkState::GENERATED);

        if (!neighbours_in_required_state) {
            // Put copy of this mesh task back onto the load task queue.
            ChunkNaiveMeshTask<MeshComparator>* mesh_task
                = new ChunkNaiveMeshTask<MeshComparator>();
            mesh_task->set_state(m_chunk, m_chunk_grid);
            task_queue->enqueue(state->producer_token, { mesh_task, true });
            return;
        }
    }

    chunk->meshing.store(ChunkState::ACTIVE, std::memory_order_release);

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

    std::unique_lock<std::shared_mutex> instance_lock;
    auto&                               instance = chunk->instance.get(instance_lock);

    // Determines if block is meshable.
    const MeshComparator meshable{};

    auto add_block = [&](BlockWorldPosition pos) {
        instance.data[instance.count++] = { f32v3(pos), f32v3(1.0f) };
    };

    Chunk* raw_chunk_ptr = chunk.get();

    std::shared_lock                    block_lock(chunk->blocks_mutex);
    std::shared_lock<std::shared_mutex> neighbour_lock;

    // TODO(Matthew): Checking block is NULL_BLOCK is wrong check really, we will have
    // transparent blocks
    //                e.g. air, to account for too.
    for (BlockIndex i = 0; i < CHUNK_VOLUME; ++i) {
        Block& voxel = chunk->blocks[i];
        if (voxel != NULL_BLOCK) {
            BlockWorldPosition block_position
                = block_world_position(chunk->position, i);

            hmem::Handle<Chunk> neighbour;

            // Check its neighbours, to decide whether to add its quads.
            // LEFT
            if (is_at_left_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j   = index_at_right_face(i);
                neighbour      = chunk->neighbours.one.left.lock();
                neighbour_lock = std::shared_lock(neighbour->blocks_mutex);
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &chunk->blocks[i - 1],
                        &chunk->blocks[i - 1],
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
                BlockIndex j   = index_at_left_face(i);
                neighbour      = chunk->neighbours.one.right.lock();
                neighbour_lock = std::shared_lock(neighbour->blocks_mutex);
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &chunk->blocks[i + 1],
                        &chunk->blocks[i + 1],
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
                BlockIndex j   = index_at_top_face(i);
                neighbour      = chunk->neighbours.one.bottom.lock();
                neighbour_lock = std::shared_lock(neighbour->blocks_mutex);
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &chunk->blocks[i - CHUNK_LENGTH],
                        &chunk->blocks[i - CHUNK_LENGTH],
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
                BlockIndex j   = index_at_bottom_face(i);
                neighbour      = chunk->neighbours.one.top.lock();
                neighbour_lock = std::shared_lock(neighbour->blocks_mutex);
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &chunk->blocks[i + CHUNK_LENGTH],
                        &chunk->blocks[i + CHUNK_LENGTH],
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
                BlockIndex j   = index_at_back_face(i);
                neighbour      = chunk->neighbours.one.front.lock();
                neighbour_lock = std::shared_lock(neighbour->blocks_mutex);
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &chunk->blocks[i - (CHUNK_AREA)],
                        &chunk->blocks[i - (CHUNK_AREA)],
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
                BlockIndex j   = index_at_front_face(i);
                neighbour      = chunk->neighbours.one.back.lock();
                neighbour_lock = std::shared_lock(neighbour->blocks_mutex);
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(
                        &chunk->blocks[i + (CHUNK_AREA)],
                        &chunk->blocks[i + (CHUNK_AREA)],
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

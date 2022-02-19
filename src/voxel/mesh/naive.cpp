#include "stdafx.h"

#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/grid.h"

#include "voxel/mesh/naive.h"

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

void hvox::ChunkNaiveMeshTask::execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) {
    // Only execute if all preloaded neighbouring chunks have at least been generated.
    auto [ _, neighbours_in_required_state ] =
            m_chunk_grid->query_all_neighbour_states(m_chunk, ChunkState::GENERATED);

    if (!neighbours_in_required_state) {
        // Put this mesh task back onto the load queue.
        ChunkNaiveMeshTask* mesh_task = new ChunkNaiveMeshTask();
        mesh_task->init(m_chunk, m_chunk_grid);
        task_queue->enqueue(state->producer_token, { mesh_task, true });
        m_chunk->pending_task.store(ChunkLoadTaskKind::MESH, std::memory_order_release);
        return;
    }

    m_chunk->instance = { nullptr, 0 };

    // TODO(Matthew): Better guess work should be possible and expand only when needed.
    //                  Maybe in addition to managing how all chunk's transformations are
    //                  stored on GPU, ChunkGrid-level should also manage this data?
    //                    This could get hard with scalings as well (as will come from
    //                    something like a greedy "meshing" algorithm).
    // TODO(Matthew):       For greedy meshing, while translations will by definition be
    //                      unique, scalings will not be, and so an index buffer could
    //                      further improve performance and also remove the difficulty
    //                      of the above TODO.
    m_chunk->instance.data = new ChunkInstanceData[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

    auto add_block = [&](BlockWorldPosition pos) {
        m_chunk->instance.data[m_chunk->instance.count++]
                                        = { f32v3(pos), f32v3(1.0f) };
    };

    // TODO(Matthew): Checking block is NULL_BLOCK is wrong check really, we will have transparent blocks
    //                e.g. air, to account for too.
    for (BlockIndex i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; ++i) {
        Block& voxel = m_chunk->blocks[i];
        if (voxel != NULL_BLOCK) {
            BlockWorldPosition block_position = block_world_position(m_chunk->position, i);

            // Check its neighbours, to decide whether to add its quads.
            // LEFT
            if (is_at_left_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_right_face(i);
                if (m_chunk->neighbours.left == nullptr || m_chunk->neighbours.left->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i - 1]  == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            }

            // RIGHT
            if (is_at_right_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_left_face(i);
                if (m_chunk->neighbours.right == nullptr || m_chunk->neighbours.right->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i + 1] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            }

            // BOTTOM
            if (is_at_bottom_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_top_face(i);
                if (m_chunk->neighbours.bottom == nullptr || m_chunk->neighbours.bottom->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i - CHUNK_SIZE] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            }

            // TOP
            if (is_at_top_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_bottom_face(i);
                if (m_chunk->neighbours.top == nullptr || m_chunk->neighbours.top->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i + CHUNK_SIZE] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            }

            // FRONT
            if (is_at_front_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_back_face(i);
                if (m_chunk->neighbours.front == nullptr || m_chunk->neighbours.front->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i - (CHUNK_SIZE * CHUNK_SIZE)] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            }

            // BACK
            if (is_at_back_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_front_face(i);
                if (m_chunk->neighbours.back == nullptr || m_chunk->neighbours.back->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i + (CHUNK_SIZE * CHUNK_SIZE)] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            }
        }
    }

    m_chunk->state.store(ChunkState::MESHED, std::memory_order_release);
}

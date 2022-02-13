#include "stdafx.h"

#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/chunk.h"
#include "voxel/grid.h"

#include "voxel/mesher.h"

static inline void add_front_quad(hvox::BlockChunkPosition pos, OUT hg::MeshData3D_32& mesh) {
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
};
static inline void add_back_quad(hvox::BlockChunkPosition pos, OUT hg::MeshData3D_32& mesh) {
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
};
static inline void add_left_quad(hvox::BlockChunkPosition pos, OUT hg::MeshData3D_32& mesh) {
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
};
static inline void add_right_quad(hvox::BlockChunkPosition pos, OUT hg::MeshData3D_32& mesh) {
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
};
static inline void add_bottom_quad(hvox::BlockChunkPosition pos, OUT hg::MeshData3D_32& mesh) {
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
};
static inline void add_top_quad(hvox::BlockChunkPosition pos, OUT hg::MeshData3D_32& mesh) {
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
};

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

void hvox::ChunkMeshTask::execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) {
    // Only execute if all preloaded neighbouring chunks have at least been generated.
    auto [ _, neighbours_in_required_state ] =
            m_chunk_grid->query_all_neighbour_states(m_chunk, ChunkState::GENERATED);

    if (!neighbours_in_required_state) {
        // Put this mesh task back onto the load queue.
        ChunkMeshTask* mesh_task = new ChunkMeshTask();
        mesh_task->init(m_chunk, m_chunk_grid);
        task_queue->enqueue(state->producer_token, { mesh_task, true });
        m_chunk->pending_task.store(ChunkLoadTaskKind::MESH, std::memory_order_release);
        return;
    }

    // TODO(Matthew): Make this indexed at least, and apply some smarter meshing generally.
    m_chunk->mesh = { nullptr, 0 };

    m_chunk->mesh.vertices = new hg::Vertex3D_32[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6];

    // TODO(Matthew): Checking block is NULL_BLOCK is wrong check really, we will have transparent blocks
    //                e.g. air, to account for too.
    for (BlockIndex i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; ++i) {
        Block& voxel = m_chunk->blocks[i];
        if (voxel != NULL_BLOCK) {
            BlockChunkPosition block_position = block_chunk_position(i);

            // Check its neighbours, to decide whether to add its quads.
            // LEFT
            if (is_at_left_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_right_face(i);
                if (m_chunk->neighbours.left == nullptr || m_chunk->neighbours.left->blocks[j] == NULL_BLOCK) {
                    add_left_quad(block_position, m_chunk->mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i - 1]  == NULL_BLOCK) {
                    add_left_quad(block_position, m_chunk->mesh);
                }
            }

            // RIGHT
            if (is_at_right_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_left_face(i);
                if (m_chunk->neighbours.right == nullptr || m_chunk->neighbours.right->blocks[j] == NULL_BLOCK) {
                    add_right_quad(block_position, m_chunk->mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i + 1] == NULL_BLOCK) {
                    add_right_quad(block_position, m_chunk->mesh);
                }
            }

            // BOTTOM
            if (is_at_bottom_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_top_face(i);
                if (m_chunk->neighbours.bottom == nullptr || m_chunk->neighbours.bottom->blocks[j] == NULL_BLOCK) {
                    add_bottom_quad(block_position, m_chunk->mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i - CHUNK_SIZE] == NULL_BLOCK) {
                    add_bottom_quad(block_position, m_chunk->mesh);
                }
            }

            // TOP
            if (is_at_top_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_bottom_face(i);
                if (m_chunk->neighbours.top == nullptr || m_chunk->neighbours.top->blocks[j] == NULL_BLOCK) {
                    add_top_quad(block_position, m_chunk->mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i + CHUNK_SIZE] == NULL_BLOCK) {
                    add_top_quad(block_position, m_chunk->mesh);
                }
            }

            // FRONT
            if (is_at_front_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_back_face(i);
                if (m_chunk->neighbours.front == nullptr || m_chunk->neighbours.front->blocks[j] == NULL_BLOCK) {
                    add_front_quad(block_position, m_chunk->mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i - (CHUNK_SIZE * CHUNK_SIZE)] == NULL_BLOCK) {
                    add_front_quad(block_position, m_chunk->mesh);
                }
            }

            // BACK
            if (is_at_back_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_front_face(i);
                if (m_chunk->neighbours.back == nullptr || m_chunk->neighbours.back->blocks[j] == NULL_BLOCK) {
                    add_back_quad(block_position, m_chunk->mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (m_chunk->blocks[i + (CHUNK_SIZE * CHUNK_SIZE)] == NULL_BLOCK) {
                    add_back_quad(block_position, m_chunk->mesh);
                }
            }
        }
    }

    m_chunk->state = ChunkState::MESHED;
}

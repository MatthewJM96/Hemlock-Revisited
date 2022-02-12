#include "stdafx.h"

#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/chunk.hpp"

#include "voxel/mesher.h"

static inline void add_front_quad(hvox::BlockChunkPosition pos, hg::MeshData3D_32 mesh) {
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
};
static inline void add_back_quad(hvox::BlockChunkPosition pos, hg::MeshData3D_32 mesh) {
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
};
static inline void add_left_quad(hvox::BlockChunkPosition pos, hg::MeshData3D_32 mesh) {
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
};
static inline void add_right_quad(hvox::BlockChunkPosition pos, hg::MeshData3D_32 mesh) {
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x,  0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
};
static inline void add_bottom_quad(hvox::BlockChunkPosition pos, hg::MeshData3D_32 mesh) {
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { {  0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y,  0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } };
    mesh.vertices[mesh.vertex_count++] = { { -0.5f + pos.x, -0.5f + pos.y, -0.5f + pos.z }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } };
};
static inline void add_top_quad(hvox::BlockChunkPosition pos, hg::MeshData3D_32 mesh) {
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

void hvox::ChunkMeshTask::execute(ChunkGenThreadState* state, ChunkGenTaskQueue*) {
    Chunk& chunk = *(state->context.chunk);

    // TODO(Matthew): Make this indexed at least, and apply some smarter meshing generally.
    hg::MeshData3D_32 chunk_mesh;

    chunk_mesh.vertices = new hg::Vertex3D_32[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6];

    // TODO(Matthew): Checking block is NULL_BLOCK is wrong check really, we will have transparent blocks
    //                e.g. air, to account for too.
    for (BlockIndex i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; ++i) {
        Block& voxel = chunk.blocks[i];
        if (voxel != NULL_BLOCK) {
            BlockChunkPosition block_position = block_chunk_position(i);

            // Check its neighbours, to decide whether to add its quads.
            // LEFT
            if (is_at_left_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_right_face(i);
                if (chunk.neighbours.left == nullptr || chunk.neighbours.left->blocks[j] == NULL_BLOCK) {
                    add_left_quad(block_position, chunk_mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (chunk.blocks[i - 1]  == NULL_BLOCK) {
                    add_left_quad(block_position, chunk_mesh);
                }
            }

            // RIGHT
            if (is_at_right_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_left_face(i);
                if (chunk.neighbours.right == nullptr || chunk.neighbours.right->blocks[j] == NULL_BLOCK) {
                    add_right_quad(block_position, chunk_mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (chunk.blocks[i + 1] == NULL_BLOCK) {
                    add_right_quad(block_position, chunk_mesh);
                }
            }

            // BOTTOM
            if (is_at_bottom_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_top_face(i);
                if (chunk.neighbours.bottom == nullptr || chunk.neighbours.bottom->blocks[j] == NULL_BLOCK) {
                    add_bottom_quad(block_position, chunk_mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (chunk.blocks[i - CHUNK_SIZE] == NULL_BLOCK) {
                    add_bottom_quad(block_position, chunk_mesh);
                }
            }

            // TOP
            if (is_at_top_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_bottom_face(i);
                if (chunk.neighbours.top == nullptr || chunk.neighbours.top->blocks[j] == NULL_BLOCK) {
                    add_top_quad(block_position, chunk_mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (chunk.blocks[i + CHUNK_SIZE] == NULL_BLOCK) {
                    add_top_quad(block_position, chunk_mesh);
                }
            }

            // FRONT
            if (is_at_front_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_back_face(i);
                if (chunk.neighbours.front == nullptr || chunk.neighbours.front->blocks[j] == NULL_BLOCK) {
                    add_front_quad(block_position, chunk_mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (chunk.blocks[i - (CHUNK_SIZE * CHUNK_SIZE)] == NULL_BLOCK) {
                    add_front_quad(block_position, chunk_mesh);
                }
            }

            // BACK
            if (is_at_back_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_front_face(i);
                if (chunk.neighbours.back == nullptr || chunk.neighbours.back->blocks[j] == NULL_BLOCK) {
                    add_back_quad(block_position, chunk_mesh);
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (chunk.blocks[i + (CHUNK_SIZE * CHUNK_SIZE)] == NULL_BLOCK) {
                    add_back_quad(block_position, chunk_mesh);
                }
            }
        }
    }

    hg::MeshHandles mesh_handles;
    hg::upload_mesh(chunk_mesh, mesh_handles, hg::MeshDataVolatility::STATIC);

    chunk.mesh_handles = mesh_handles;
    chunk.state        = ChunkState::MESHED;

    delete[] chunk_mesh.vertices;
}
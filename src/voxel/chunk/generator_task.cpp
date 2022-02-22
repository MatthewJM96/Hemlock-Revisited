#include "stdafx.h"

#include "voxel/block.hpp"
#include "voxel/chunk.h"
#include "voxel/chunk/mesh/greedy_task.h"

#include "voxel/chunk/generator_task.h"

void hvox::ChunkGenerationTask::execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) {
    m_chunk->gen_task_active.store(true, std::memory_order_release);

    // BlockWorldPosition chunk_position = block_world_position(m_chunk->position, 0);

    // if (chunk_position.y < 0) {
    //     set_blocks(m_chunk, BlockChunkPosition{0}, BlockChunkPosition{CHUNK_SIZE - 1}, Block{1});
    // }

    for (auto y = 0; y < CHUNK_SIZE; y += 2) {
        const ui8 LEFT  = 14;
        const ui8 RIGHT = 16;
        set_blocks(m_chunk, BlockChunkPosition{0, y, 0}, BlockChunkPosition{LEFT, y, LEFT}, Block{1});
        set_blocks(m_chunk, BlockChunkPosition{RIGHT, y, RIGHT}, BlockChunkPosition{CHUNK_SIZE - 1, y, CHUNK_SIZE - 1}, Block{1});
    }

    // for (auto x = 0; x < CHUNK_SIZE; ++x) {
    //     for (auto z = 0; z < CHUNK_SIZE; ++z) {
    //         // Do stuff...
    //     }
    // }

    m_chunk->state.store(ChunkState::GENERATED, std::memory_order_release);

    // if (task_queue) {
    ChunkGreedyMeshTask* mesh_task = new ChunkGreedyMeshTask();
    mesh_task->init(m_chunk, m_chunk_grid);
    m_chunk->pending_task.store(ChunkLoadTaskKind::MESH, std::memory_order_release);
    task_queue->enqueue(state->producer_token, { mesh_task, true });
    // }

    m_chunk->gen_task_active.store(false, std::memory_order_release);
}

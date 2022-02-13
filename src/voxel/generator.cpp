#include "stdafx.h"

#include "voxel/block.hpp"
#include "voxel/chunk.h"
#include "voxel/mesher.h"

#include "voxel/generator.h"

void hvox::ChunkGenerationTask::execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) {
    // TODO(Matthew): Will we use gen_task_active anywhere? And if we do, does that need to
    //                be atomic to assure what we want it to be achieving?
    m_chunk->gen_task_active = true;

    BlockWorldPosition chunk_position = block_world_position(m_chunk->position, 0);

    if (chunk_position.y < 0) {
        set_blocks(m_chunk, BlockChunkPosition{0}, BlockChunkPosition{CHUNK_SIZE}, Block{1});
    }

    // for (auto x = 0; x < CHUNK_SIZE; ++x) {
    //     for (auto z = 0; z < CHUNK_SIZE; ++z) {
    //         // Do stuff...
    //     }
    // }

    m_chunk->state.store(ChunkState::GENERATED, std::memory_order_release);

    // if (task_queue) {
    ChunkMeshTask* mesh_task = new ChunkMeshTask();
    mesh_task->init(m_chunk, m_chunk_grid);
    m_chunk->pending_task.store(ChunkLoadTaskKind::MESH, std::memory_order_release);
    task_queue->enqueue(state->producer_token, { mesh_task, true });
    // }

    m_chunk->gen_task_active = false;
}

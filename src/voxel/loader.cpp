#include "stdafx.h"

#include "voxel/block.hpp"
#include "voxel/chunk.h"
#include "voxel/mesher.h"

#include "voxel/loader.h"

void hvox::ChunkLoadTask::execute(ChunkGenThreadState* state, ChunkGenTaskQueue* task_queue) {
    m_chunk->gen_task_active = true;

    BlockWorldPosition chunk_position = block_world_position(m_chunk->position, 0);

    if (chunk_position.y < 0) {
        set_blocks(m_chunk, BlockChunkPosition{0}, BlockChunkPosition{CHUNK_SIZE}, Block{1});
    }

    ChunkMeshTask* mesh_task = new ChunkMeshTask();
    mesh_task->init(m_chunk);
    task_queue->enqueue(state->producer_token, { mesh_task, true });

    // for (auto x = 0; x < CHUNK_SIZE; ++x) {
    //     for (auto z = 0; z < CHUNK_SIZE; ++z) {
    //         // Do stuff...
    //     }
    // }

    m_chunk->gen_task_active = false;
}

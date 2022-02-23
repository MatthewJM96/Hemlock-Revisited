#include "stdafx.h"

#include "voxel/block.hpp"
#include "voxel/chunk.h"
#include "voxel/chunk/mesh/greedy_task.h"

#include "voxel/chunk/generator_task.h"

void hvox::ChunkGenerationTask::execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) {
    m_chunk->gen_task_active.store(true, std::memory_order_release);

    // TODO(Matthew): double pointer hop, we can do better, but need we?
    //                  tbh yes just on the grounds it is inconsistent with
    //                  how we are doing different meshing strategies.
    auto strategy = reinterpret_cast<ChunkGenerationStrategy*>(m_strategy);

    (*strategy)(m_chunk->blocks, m_chunk->position);

    m_chunk->state.store(ChunkState::GENERATED, std::memory_order_release);

    ChunkGreedyMeshTask* mesh_task = new ChunkGreedyMeshTask();
    mesh_task->init(m_chunk, m_chunk_grid, nullptr);
    m_chunk->pending_task.store(ChunkLoadTaskKind::MESH, std::memory_order_release);
    task_queue->enqueue(state->producer_token, { mesh_task, true });

    m_chunk->gen_task_active.store(false, std::memory_order_release);
}

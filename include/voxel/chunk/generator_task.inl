#include "voxel/block.hpp"
#include "voxel/chunk.h"
// #include "voxel/chunk/mesh/greedy_task.h"
#include "voxel/chunk/mesh/naive_task.h"

template <hvox::ChunkGenerationStrategy GenerationStrategy>   
void hvox::ChunkGenerationTask<GenerationStrategy>::execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) {
    m_chunk->gen_task_active.store(true, std::memory_order_release);

    GenerationStrategy()(m_chunk->blocks, m_chunk->position);

    m_chunk->state.store(ChunkState::GENERATED, std::memory_order_release);

    // TODO(Matthew): graph system to chain tasks?
    // // ChunkGreedyMeshTask* mesh_task = new ChunkGreedyMeshTask();
    // ChunkNaiveMeshTask* mesh_task = new ChunkNaiveMeshTask();
    // mesh_task->init(m_chunk, m_chunk_grid, nullptr);
    // m_chunk->pending_task.store(ChunkLoadTaskKind::MESH, std::memory_order_release);
    // task_queue->enqueue(state->producer_token, { mesh_task, true });

    m_chunk->gen_task_active.store(false, std::memory_order_release);
}

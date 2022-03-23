#include "voxel/block.hpp"
#include "voxel/chunk.h"
// #include "voxel/chunk/mesh/greedy_task.hpp"
#include "voxel/chunk/mesh/naive_task.hpp"

template <hvox::ChunkGenerationStrategy GenerationStrategy>   
bool hvox::ChunkGenerationTask<GenerationStrategy>::run_task(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) {
    m_chunk->gen_task_active.store(true, std::memory_order_release);

    const GenerationStrategy generate{};

    generate(m_chunk);

    m_chunk->state.store(ChunkState::GENERATED, std::memory_order_release);

    m_chunk->gen_task_active.store(false, std::memory_order_release);

    // TODO(Matthew): Set next task if chunk unload is false? Or else set that
    //                between this task and next, but would need adjusting
    //                workflow.
    m_chunk->pending_task.store(ChunkLoadTaskKind::NONE, std::memory_order_release);

    return !m_chunk->unload.load(std::memory_order_acquire);
}

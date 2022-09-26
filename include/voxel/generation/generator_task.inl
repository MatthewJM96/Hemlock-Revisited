template <hvox::ChunkGenerationStrategy GenerationStrategy>
void hvox::ChunkGenerationTask<GenerationStrategy>::execute(ChunkLoadThreadState*, ChunkTaskQueue*) {
    auto chunk = m_chunk.lock();

    if (chunk == nullptr) return;

    chunk->gen_task_active.store(true, std::memory_order_release);

    const GenerationStrategy generate{};

    generate(chunk);

    chunk->state.store(ChunkState::GENERATED, std::memory_order_release);

    chunk->gen_task_active.store(false, std::memory_order_release);

    chunk->on_load();

    // TODO(Matthew): Set next task if chunk unload is false? Or else set that
    //                between this task and next, but would need adjusting
    //                workflow.
    chunk->pending_task.store(ChunkTaskKind::NONE, std::memory_order_release);
}

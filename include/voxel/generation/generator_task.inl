template <hvox::ChunkGenerationStrategy GenerationStrategy>
void hvox::ChunkGenerationTask<
    GenerationStrategy>::execute(ChunkLoadThreadState*, ChunkTaskQueue*) {
    auto chunk_grid = m_chunk_grid.lock();
    if (chunk_grid == nullptr) return;

    if (!chunk_grid->registry()->valid(m_chunk)) return;

    chunk->generation.store(ChunkState::ACTIVE, std::memory_order_release);

    const GenerationStrategy generate{};

    generate(chunk);

    chunk->generation.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk->on_load();
}

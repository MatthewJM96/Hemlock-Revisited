template <hvox::ChunkGenerationStrategy GenerationStrategy>
void hvox::ChunkGenerationTask<
    GenerationStrategy>::execute(ChunkLoadThreadState*, ChunkTaskQueue*) {
    auto chunk = m_chunk.lock();

    if (chunk == nullptr) return;

    chunk->generation.store(ChunkState::ACTIVE, std::memory_order_release);

    const GenerationStrategy generate{};

    generate(chunk);

    chunk->generation.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk->on_load();
}

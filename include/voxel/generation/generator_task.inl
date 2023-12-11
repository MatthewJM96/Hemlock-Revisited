template <hvox::ChunkGenerationStrategy GenerationStrategy>
bool hvox::ChunkGenerationTask<GenerationStrategy>::execute() {
    auto chunk = m_chunk.lock();

    if (chunk == nullptr) return true;

    chunk->generation.store(ChunkState::ACTIVE, std::memory_order_release);

    const GenerationStrategy generate{};

    generate(chunk);

    chunk->generation.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk->on_load();

    return true;
}

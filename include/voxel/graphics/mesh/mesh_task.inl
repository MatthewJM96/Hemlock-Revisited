template <hvox::ChunkMeshStrategy MeshStrategy>
bool hvox::ChunkMeshTask<MeshStrategy>::execute(ChunkThreadState*, hthread::QueueDelegate*) {
    auto chunk_grid = m_chunk_grid.lock();
    if (chunk_grid == nullptr) return true;

    auto chunk = m_chunk.lock();
    if (chunk == nullptr) return true;

    const MeshStrategy mesh{};

    if (!mesh.can_run(chunk_grid, chunk)) {
        return false;
    }

    chunk->meshing.store(ChunkState::ACTIVE, std::memory_order_release);

    mesh(chunk_grid, chunk);

    chunk->meshing.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk->on_mesh_change();

    return true;
}

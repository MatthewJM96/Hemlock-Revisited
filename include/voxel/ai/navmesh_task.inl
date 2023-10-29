template <hvox::ai::ChunkNavmeshStrategy NavmeshStrategy>
void hvox::ai::ChunkNavmeshTask<
    NavmeshStrategy>::execute(ChunkLoadThreadState*, ChunkTaskQueue*) {
    auto chunk_grid = m_chunk_grid.lock();
    if (chunk_grid == nullptr) return;

    if (!chunk_grid->registry()->valid(m_chunk)) return;

    auto& chunk_navmesh = chunk_grid->registry()->get<ChunkNavmeshComponent>(m_chunk);

    const NavmeshStrategy navmesh{};

    chunk_navmesh.navmeshing.store(ChunkState::ACTIVE, std::memory_order_release);
    chunk_navmesh.bulk_navmeshing.store(ChunkState::ACTIVE, std::memory_order_release);

    navmesh.do_bulk(chunk_grid, chunk);

    chunk_navmesh.bulk_navmeshing.store(
        ChunkState::COMPLETE, std::memory_order_release
    );

    navmesh.do_stitch(chunk_grid, chunk);

    chunk_navmesh.navmeshing.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk_navmesh.on_navmesh_change();
}

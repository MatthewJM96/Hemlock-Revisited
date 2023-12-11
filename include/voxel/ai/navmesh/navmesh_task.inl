#include "voxel/chunk/chunk.h"
#include "voxel/chunk/state.hpp"

template <hvox::ai::ChunkNavmeshStrategy NavmeshStrategy>
bool hvox::ai::ChunkNavmeshTask<NavmeshStrategy>::execute() {
    auto chunk_grid = m_chunk_grid.lock();
    if (chunk_grid == nullptr) return true;

    auto chunk = m_chunk.lock();
    if (chunk == nullptr) return true;

    const NavmeshStrategy navmesh{};

    chunk->navmeshing.store(ChunkState::ACTIVE, std::memory_order_release);
    chunk->bulk_navmeshing.store(ChunkState::ACTIVE, std::memory_order_release);

    navmesh.do_bulk(chunk_grid, chunk);

    chunk->bulk_navmeshing.store(ChunkState::COMPLETE, std::memory_order_release);

    navmesh.do_stitch(chunk_grid, chunk);

    chunk->navmeshing.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk->on_navmesh_change();

    return true;
}

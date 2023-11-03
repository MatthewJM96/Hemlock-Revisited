#include "stdafx.h"

#include "voxel/ai/navmesh/navmesh_manager.h"

void hvox::ai::ChunkNavmeshManager::init(hmem::Handle<ChunkNavmeshPager> navmesh_pager
) {
    m_navmesh_pager = navmesh_pager;
}

void hvox::ai::ChunkNavmeshManager::dispose() {
    free_buffer();

    m_navmesh_pager = nullptr;
}

void hvox::ai::ChunkNavmeshManager::generate_buffer() {
    std::unique_lock lock(m_mutex);

    if (!m_resource) m_resource = m_navmesh_pager->get_page();
}

void hvox::ai::ChunkNavmeshManager::free_buffer() {
    std::unique_lock lock(m_mutex);

    if (m_resource) m_navmesh_pager->free_page(m_resource);
    m_resource = nullptr;
}

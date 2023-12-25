#include "stdafx.h"

#include "voxel/voxel_manager.h"

void hvox::VoxelManager::init(hmem::Handle<ChunkVoxelPager> voxel_pager) {
    m_voxel_pager = voxel_pager;

    generate_buffer();
}

void hvox::VoxelManager::dispose() {
    free_buffer();

    m_voxel_pager = nullptr;
}

void hvox::VoxelManager::generate_buffer() {
    std::unique_lock lock(m_mutex);

    if (!m_resource) m_resource = m_voxel_pager->get_page();
}

void hvox::VoxelManager::free_buffer() {
    std::unique_lock lock(m_mutex);

    if (m_resource) m_voxel_pager->free_page(m_resource);
    m_resource = nullptr;
}

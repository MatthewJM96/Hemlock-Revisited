#include "voxel/graphics/mesh/instance_manager.h"
#include "stdafx.h"

void hvox::ChunkInstanceManager::init(hmem::Handle<ChunkInstanceDataPager> data_pager) {
    m_data_pager = data_pager;
}

void hvox::ChunkInstanceManager::dispose() {
    free_buffer();

    m_data_pager = nullptr;
}

void hvox::ChunkInstanceManager::generate_buffer() {
    std::unique_lock lock(m_mutex);

    m_resource.count = 0;
    if (!m_resource.data) m_resource.data = m_data_pager->get_page();
}

void hvox::ChunkInstanceManager::free_buffer() {
    std::unique_lock lock(m_mutex);

    m_resource.count = 0;
    if (m_resource.data) m_data_pager->free_page(m_resource.data);
    m_resource.data = nullptr;
}

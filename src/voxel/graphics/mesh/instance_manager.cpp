#include "voxel/graphics/mesh/instance_manager.h"
#include "stdafx.h"

hvox::ChunkInstanceManager::ChunkInstanceManager() :
    m_instance({ nullptr, 0 }) { /* Empty. */
}

void hvox::ChunkInstanceManager::init(hmem::Handle<ChunkInstanceDataPager> data_pager
) {
    m_data_pager = data_pager;
}

void hvox::ChunkInstanceManager::dispose() {
    if (m_instance.data) m_data_pager->free_page(m_instance.data);

    m_instance.data  = nullptr;
    m_instance.count = 0;
    m_data_pager     = nullptr;
}

hvox::ChunkInstance&
hvox::ChunkInstanceManager::get(std::unique_lock<std::shared_mutex>& lock) {
    lock = std::unique_lock(m_mutex);
    return m_instance;
}

const hvox::ChunkInstance&
hvox::ChunkInstanceManager::get(std::shared_lock<std::shared_mutex>& lock) {
    lock = std::shared_lock(m_mutex);
    return m_instance;
}

void hvox::ChunkInstanceManager::generate_buffer() {
    std::unique_lock lock(m_mutex);

    m_instance.count = 0;
    if (!m_instance.data) m_instance.data = m_data_pager->get_page();
}

void hvox::ChunkInstanceManager::free_buffer() {
    std::unique_lock lock(m_mutex);

    m_instance.count = 0;
    if (m_instance.data) m_data_pager->free_page(m_instance.data);
    m_instance.data = nullptr;
}

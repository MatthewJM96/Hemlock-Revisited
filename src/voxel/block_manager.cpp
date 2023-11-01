#include "stdafx.h"

#include "voxel/block_manager.h"

void hvox::ChunkInstanceManager::init(hmem::Handle<ChunkBlockPager> block_pager) {
    m_block_pager = block_pager;

    generate_buffer();
}

void hvox::ChunkInstanceManager::dispose() {
    free_buffer();

    m_block_pager = nullptr;
}

void hvox::ChunkInstanceManager::generate_buffer() {
    std::unique_lock lock(m_mutex);

    m_resource.count = 0;
    if (!m_resource.data) m_resource.data = m_block_pager->get_page();
}

void hvox::ChunkInstanceManager::free_buffer() {
    std::unique_lock lock(m_mutex);

    m_resource.count = 0;
    if (m_resource.data) m_block_pager->free_page(m_resource.data);
    m_resource.data = nullptr;
}

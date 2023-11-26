#include "stdafx.h"

#include "voxel/block_manager.h"

void hvox::BlockManager::init(hmem::Handle<ChunkBlockPager> block_pager) {
    m_block_pager = block_pager;

    generate_buffer();
}

void hvox::BlockManager::dispose() {
    free_buffer();

    m_block_pager = nullptr;
}

void hvox::BlockManager::generate_buffer() {
    std::unique_lock lock(m_mutex);

    if (!m_resource) m_resource = m_block_pager->get_page();
}

void hvox::BlockManager::free_buffer() {
    std::unique_lock lock(m_mutex);

    if (m_resource) m_block_pager->free_page(m_resource);
    m_resource = nullptr;
}

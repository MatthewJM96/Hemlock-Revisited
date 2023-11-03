#include "stdafx.h"

#include "voxel/ai/navmesh/navmesh_manager.h"

void hvox::ai::ChunkNavmeshManager::generate_buffer() {
    hmem::UniqueResourceLock lock;
    auto&                    resource = get(lock);

    resource.count = 1;
    if (!resource.data) {
        resource.data = m_pager->get_page();

        new (resource.data) ChunkNavmesh{};
    }
}

void hvox::ai::ChunkNavmeshManager::free_buffer() {
    hmem::UniqueResourceLock lock;
    auto&                    resource = get(lock);

    resource.data->~ChunkNavmesh();

    resource.count = 0;
    if (resource.data) m_pager->free_page(resource.data);
    resource.data = nullptr;
}

#include "stdafx.h"

#include "voxel/chunk/component/core.hpp"

#include "voxel/chunk/builder.h"

void hvox::ChunkBuilder::init(hmem::Handle<ChunkBlockPager> block_pager) {
    m_block_pager = block_pager;
}

entt::entity
hvox::ChunkBuilder::build(entt::registry& registry, ChunkGridPosition chunk_position) {
    entt::entity entity = registry.create();

    registry.emplace<ChunkCoreComponent>(entity, chunk_position, m_block_pager);

    for (auto& builder : m_builders) {
        builder(registry, entity);
    }

    return entity;
}

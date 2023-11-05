#include "stdafx.h"

#include "graphics/mesh.h"

#include "voxel/block.hpp"
#include "voxel/chunk/component/core.hpp"

#include "voxel/chunk/grid.h"

void hvox::ChunkGrid::init(
    hmem::WeakHandle<ChunkGrid>  self,
    ChunkBuilderBase*            chunk_builder,
    hmem::Handle<entt::registry> chunk_registry /* = nullptr*/
) {
    m_self = self;

    if (chunk_registry) {
        m_chunk_registry = chunk_registry;
    } else {
        m_chunk_registry = hmem::make_handle<entt::entity>();
    }

    m_block_pager    = hmem::make_handle<ChunkBlockPager>();
    m_instance_pager = hmem::make_handle<ChunkInstanceDataPager>();
    m_navmesh_pager  = hmem::make_handle<ai::ChunkNavmeshPager>();

    // TODO(Matthew): smarter setting of page size - maybe should be dependent on draw
    // distance. m_renderer.init(20, 2);
    m_renderer.init(5, 2);
}

void hvox::ChunkGrid::dispose() {
    m_thread_pool.dispose();

    m_renderer.dispose();

    for (auto& [id, chunk] : m_chunks) {
        chunk.lock.release();
    }
    m_chunk_registry = nullptr;
}

void hvox::ChunkGrid::update(FrameTime time) {
    // TODO(Matthew): might not want to even do this here?
    // for (auto [id, chunk] : m_chunks) {
    //     auto& chunk_core = m_chunk_registry->registry.get<ChunkCore>(chunk.entity);
    //     // lock chunk_core
    //     chunk_core.update(time);
    // }

    m_renderer.update(time);
}

void hvox::ChunkGrid::draw(FrameTime time) {
    m_renderer.draw(time);
}

void hvox::ChunkGrid::set_render_distance(ui32 render_distance) {
    // TODO(Matthew): Allow chunk grids with non-standard render shapes?
    ui32 chunks_in_render_distance
        = render_distance * render_distance * render_distance;

    on_render_distance_change({
        {m_render_distance, m_chunks_in_render_distance},
        {  render_distance,   chunks_in_render_distance}
    });

    m_render_distance           = render_distance;
    m_chunks_in_render_distance = chunks_in_render_distance;
}

bool hvox::ChunkGrid::load_chunks(
    ChunkGridPosition* chunk_positions, ui32 chunk_count
) {
    bool any_chunk_failed = false;

    for (ui32 i = 0; i < chunk_count; ++i) {
        any_chunk_failed = load_chunk_at(chunk_positions[i]);

        if (any_chunk_failed) break;
    }

    return any_chunk_failed;
}

bool hvox::ChunkGrid::load_chunk(ChunkGridPosition chunk_position) {
    auto it = m_chunks.find(chunk_position.id);
    if (it != m_chunks.end()) return false;

    establish_chunk_neighbours(chunk_core);

    registry.emplace<hecs::ProtectedComponent<ChunkCoreComponent>>(
        chunk_entity, deletor, std::move(core_component)
    );

    m_chunks[chunk_position.id] = ChunkAndDeletor{
        chunk_entity, hecs::ProtectedComponentLock(deletor), deletor
    };

    // TODO(Matthew): not clear yet but probably not how we want to do this.
    m_renderer.add_chunk(chunk);

    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return false;

    auto& [id, chunk] = *it;

    // TODO(Matthew): acquire lock. but probably not as this stuff should be done via
    //                events...

    // If chunk is in the process of being generated, we don't
    // need to add it to the queue again.
    ChunkState chunk_state = ChunkState::NONE;
    if (!chunk->generation.compare_exchange_strong(chunk_state, ChunkState::PENDING)) {
        return false;
    }

    auto task = m_build_load_or_generate_task();
    task->set_state(chunk, m_self);
    m_thread_pool.add_task({ task, true });

    return true;
}

bool hvox::ChunkGrid::unload_chunk(ChunkGridPosition chunk_position) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return false;

    (*it).second->on_unload();

    if (handle) {
        *handle = (*it).second;
    }

    // TODO(Matthew): wherever unloaded, we need to make sure we get IO right,
    //                as chunk will "float" and something could act as if that
    //                floating data is true even as the chunk is reloaded from
    //                disk with a different truth.

    m_chunks.erase(it);

    return true;
}

hmem::Handle<hvox::Chunk> hvox::ChunkGrid::chunk(ChunkID id) {
    auto it = m_chunks.find(id);

    if (it == m_chunks.end()) return nullptr;

    return it->second;
}

void hvox::ChunkGrid::establish_chunk_neighbours(entt::entity chunk) {
    ChunkGridPosition neighbour_position;

    // Update neighbours with info of new chunk.
    // LEFT
    neighbour_position   = chunk.position;
    neighbour_position.x -= 1;
    auto it              = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        auto& [id, neighbour]              = *it;
        chunk.neighbours.one.left          = neighbour.entity;
        (*it).second->neighbours.one.right = chunk;
    } else {
        chunk.neighbours.one.left = hmem::WeakHandle<Chunk>();
    }

    // RIGHT
    neighbour_position   = chunk.position;
    neighbour_position.x += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk.neighbours.one.right        = (*it).second;
        (*it).second->neighbours.one.left = chunk;
    } else {
        chunk.neighbours.one.right = hmem::WeakHandle<Chunk>();
    }

    // TOP
    neighbour_position   = chunk.position;
    neighbour_position.y += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk.neighbours.one.top            = (*it).second;
        (*it).second->neighbours.one.bottom = chunk;
    } else {
        chunk.neighbours.one.top = hmem::WeakHandle<Chunk>();
    }

    // BOTTOM
    neighbour_position   = chunk.position;
    neighbour_position.y -= 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk.neighbours.one.bottom      = (*it).second;
        (*it).second->neighbours.one.top = chunk;
    } else {
        chunk.neighbours.one.bottom = hmem::WeakHandle<Chunk>();
    }

    // FRONT
    neighbour_position   = chunk.position;
    neighbour_position.z += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk.neighbours.one.front        = (*it).second;
        (*it).second->neighbours.one.back = chunk;
    } else {
        chunk.neighbours.one.front = hmem::WeakHandle<Chunk>();
    }

    // BACK
    neighbour_position   = chunk.position;
    neighbour_position.z -= 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk.neighbours.one.back          = (*it).second;
        (*it).second->neighbours.one.front = chunk;
    } else {
        chunk.neighbours.one.back = hmem::WeakHandle<Chunk>();
    }
}

#include "stdafx.h"

#include "graphics/mesh.h"

#include "voxel/block.hpp"
#include "voxel/chunk/component/core.hpp"
#include "voxel/chunk/component/lodable.hpp"
#include "voxel/chunk/component/mesh.hpp"
#include "voxel/chunk/component/navmesh.hpp"
#include "voxel/chunk/grid.h"

void hvox::ChunkTask::set_state(
    hmem::WeakHandle<Chunk> chunk, hmem::WeakHandle<ChunkGrid> chunk_grid
) {
    m_chunk      = chunk;
    m_chunk_grid = chunk_grid;
}

hvox::ChunkGrid::ChunkGrid() :
    // TODO(Matthew): both of these are rather inefficient, we remesh literally every
    // block change, rather than
    //                once per update loop, likewise with chunk loads.
    // TODO(Matthew): as these are called in threadpool threads, we'd like them to
    //                be able to use the appropriate producer tokens.
    handle_chunk_load(Delegate<void(Sender)>{ [&](Sender sender) {
        hmem::WeakHandle<Chunk> handle = sender.get_handle<Chunk>();

        auto chunk = handle.lock();
        // If chunk is nullptr, then there's no point
        // handling the block change as we will have
        // an unload event for this chunk.
        if (chunk == nullptr) return;

        auto mesh_task = m_build_mesh_task();
        mesh_task->set_state(chunk, m_self);
        m_thread_pool.threadsafe_add_task({ mesh_task, true });

        if (m_build_navmesh_task) {
            auto navmesh_task = m_build_navmesh_task();
            navmesh_task->set_state(chunk, m_self);
            m_thread_pool.threadsafe_add_task({ navmesh_task, true });
        }
    } }),
    // TODO(Matthew): handle bulk block change too.
    // TODO(Matthew): right now we remesh even if block change is cancelled.
    //                perhaps we can count changes and if at least 1 change
    //                on end of update loop then schedule remesh, only
    //                incrementing if block change actually occurs
    //                  i.e. stop queuing here.
    handle_block_change(Delegate<bool(Sender, BlockChangeEvent)>{
        [&](Sender sender, BlockChangeEvent) {
            hmem::WeakHandle<Chunk> handle = sender.get_handle<Chunk>();

            auto chunk = handle.lock();
            // If chunk is nullptr, then there's no point
            // handling the block change as we will have
            // an unload event for this chunk.
            if (chunk == nullptr) return true;

            auto mesh_task = m_build_mesh_task();
            mesh_task->set_state(chunk, m_self);
            m_thread_pool.add_task({ mesh_task, true });

            if (m_build_navmesh_task) {
                auto navmesh_task = m_build_navmesh_task();
                navmesh_task->set_state(chunk, m_self);
                m_thread_pool.threadsafe_add_task({ navmesh_task, true });
            }

            return false;
        } }) {
    // Empty.
}

void hvox::ChunkGrid::init(
    hmem::WeakHandle<ChunkGrid>          self,
    ui32                                 render_distance,
    ui32                                 thread_count,
    hmem::Handle<entt::registry>         chunk_registry /*= nullptr*/,
    hmem::Handle<ChunkBlockPager>        block_pager /*= nullptr*/,
    hmem::Handle<ChunkInstanceDataPager> instance_data_pager /*= nullptr*/
) {
    m_self = self;

    m_render_distance = render_distance;
    // TODO(Matthew): let shaping of rendering be defined by user?
    m_chunks_in_render_distance = render_distance * render_distance * render_distance;

    m_thread_pool.init(thread_count);

    if (chunk_registry) {
        m_chunk_registry.init(std::move(chunk_registry));
    } else {
        m_chunk_registry.init(hmem::make_handle<entt::registry>());
    }

    if (block_pager) {
        m_block_pager = block_pager;
    } else {
        m_block_pager = hmem::make_handle<ChunkBlockPager>();
    }

    if (instance_data_pager) {
        m_instance_data_pager = instance_data_pager;
    } else {
        m_instance_data_pager = hmem::make_handle<ChunkInstanceDataPager>();
    }

    // TODO(Matthew): move this out of here - can one renderer be made responsible for
    //                all chunk grids with mesh component?
    // TODO(Matthew): smarter setting of page size - maybe should be dependent on draw
    // distance. m_renderer.init(20, 2);
    m_renderer.init(5, 2);
}

void hvox::ChunkGrid::dispose() {
    m_thread_pool.dispose();

    m_renderer.dispose();
}

void hvox::ChunkGrid::update(FrameTime time) {
    // for (auto chunk : m_chunks) {
    //     // TODO(Matthew): probably going to update in a different pattern so can
    //     probs
    //     //                expect to remove this...
    //     // chunk.second->update(time);
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
        any_chunk_failed = any_chunk_failed || load_chunk_at(chunk_positions[i]);
    }

    return any_chunk_failed;
}

bool hvox::ChunkGrid::preload_chunk_at(ChunkGridPosition chunk_position) {
    if (m_chunks.contains(chunk_position.id)) return false;

    // TODO(Matthew): this should actually be done in a way that is less fixed, but for
    //                now hardcoding components is fine.

    auto chunk = m_chunk_registry->create();

    auto& chunk_core
        = m_chunk_registry->emplace<ChunkCoreComponent>(chunk, m_block_pager);
    chunk_core.position = chunk_position;

    m_chunk_registry->emplace<ChunkLODComponent>(chunk);
    m_chunk_registry->emplace<ChunkMeshComponent>(chunk, m_instance_data_pager);
    m_chunk_registry->emplace<ChunkNavmeshComponent>(chunk);

    chunk_core.on_load         += &handle_chunk_load;
    chunk_core.on_block_change += &handle_block_change;

    establish_chunk_neighbours(chunk, chunk_core);

    m_chunks[chunk_position.id] = chunk;

    m_renderer.add_chunk(chunk);

    return true;
}

bool hvox::ChunkGrid::load_chunk_at(ChunkGridPosition chunk_position) {
    preload_chunk_at(chunk_position);

    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return false;

    auto  chunk      = (*it).second;
    auto& chunk_core = m_chunk_registry->get<ChunkCoreComponent>(chunk);

    // If chunk is in the process of being generated, we don't
    // need to add it to the queue again.
    ChunkState chunk_state = ChunkState::NONE;
    if (!chunk_core.generation.compare_exchange_strong(
            chunk_state, ChunkState::PENDING
        ))
    {
        return false;
    }

    // TODO(Matthew): between loading and unloading, as tasks will be running that query
    //                and even modify chunk data on threads elsewhere, sat in queues in
    //                a way that is not tracked, we need to find a way as with smart ptr
    //                to get guarantees of no UB in those tasks.

    auto task = m_build_load_or_generate_task();
    task->set_state(chunk, m_self);
    m_thread_pool.add_task({ task, true });

    return true;
}

bool hvox::ChunkGrid::unload_chunk_at(
    ChunkGridPosition chunk_position, hmem::WeakHandle<Chunk>* handle /*= nullptr*/
) {
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

void hvox::ChunkGrid::establish_chunk_neighbours(
    entt::entity chunk, ChunkCoreComponent& chunk_core
) {
    ChunkGridPosition neighbour_position;

    // Update neighbours with info of new chunk.
    // LEFT
    neighbour_position   = chunk_core.position;
    neighbour_position.x -= 1;
    auto it              = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk_core.neighbours.one.left = (*it).second;

        auto& neighbour_chunk = m_chunk_registry->get<ChunkCoreComponent>((*it).second);
        neighbour_chunk_core.neighbours.one.right = chunk;
    } else {
        chunk_core.neighbours.one.left = entt::null;
    }

    // RIGHT
    neighbour_position   = chunk_core.position;
    neighbour_position.x += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk_core.neighbours.one.right = (*it).second;

        auto& neighbour_chunk = m_chunk_registry->get<ChunkCoreComponent>((*it).second);
        neighbour_chunk_core.neighbours.one.left = chunk;
    } else {
        chunk_core.neighbours.one.right = entt::null;
    }

    // TOP
    neighbour_position   = chunk_core.position;
    neighbour_position.y += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk_core.neighbours.one.top = (*it).second;

        auto& neighbour_chunk = m_chunk_registry->get<ChunkCoreComponent>((*it).second);
        neighbour_chunk_core.neighbours.one.bottom = chunk;
    } else {
        chunk_core.neighbours.one.top = entt::null;
    }

    // BOTTOM
    neighbour_position   = chunk_core.position;
    neighbour_position.y -= 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk_core.neighbours.one.bottom = (*it).second;

        auto& neighbour_chunk = m_chunk_registry->get<ChunkCoreComponent>((*it).second);
        neighbour_chunk_core.neighbours.one.top = chunk;
    } else {
        chunk_core.neighbours.one.bottom = entt::null;
    }

    // FRONT
    neighbour_position   = chunk_core.position;
    neighbour_position.z -= 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk_core.neighbours.one.front = (*it).second;

        auto& neighbour_chunk = m_chunk_registry->get<ChunkCoreComponent>((*it).second);
        neighbour_chunk_core.neighbours.one.back = chunk;
    } else {
        chunk_core.neighbours.one.front = entt::null;
    }

    // BACK
    neighbour_position   = chunk_core.position;
    neighbour_position.z += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk_core.neighbours.one.back = (*it).second;

        auto& neighbour_chunk = m_chunk_registry->get<ChunkCoreComponent>((*it).second);
        neighbour_chunk_core.neighbours.one.front = chunk;
    } else {
        chunk_core.neighbours.one.back = entt::null;
    }
}

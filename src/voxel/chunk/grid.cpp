#include "stdafx.h"

#include "graphics/mesh.h"

#include "voxel/chunk/grid.h"
#include "voxel/state.hpp"

void hvox::ChunkTask::set_state(
    hmem::WeakHandle<Chunk> chunk, hmem::WeakHandle<ChunkGrid> chunk_grid
) {
    m_chunk      = chunk;
    m_chunk_grid = chunk_grid;
}

hvox::ChunkGrid::ChunkGrid() :
    // TODO(Matthew): both of these are rather inefficient, we remesh literally every
    // voxel change, rather than
    //                once per update loop, likewise with chunk loads.
    // TODO(Matthew): as these are called in threadpool threads, we'd like them to
    //                be able to use the appropriate producer tokens.
    handle_chunk_load(Delegate<void(Sender)>{ [&](Sender sender) {
        hmem::WeakHandle<Chunk> handle = sender.get_handle<Chunk>();

        auto chunk = handle.lock();
        // If chunk is nullptr, then there's no point
        // handling the voxel change as we will have
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
    // TODO(Matthew): handle bulk voxel change too.
    // TODO(Matthew): right now we remesh even if voxel change is cancelled.
    //                perhaps we can count changes and if at least 1 change
    //                on end of update loop then schedule remesh, only
    //                incrementing if voxel change actually occurs
    //                  i.e. stop queuing here.
    handle_voxel_change(Delegate<bool(Sender, VoxelChangeEvent)>{
        [&](Sender sender, VoxelChangeEvent) {
            hmem::WeakHandle<Chunk> handle = sender.get_handle<Chunk>();

            auto chunk = handle.lock();
            // If chunk is nullptr, then there's no point
            // handling the voxel change as we will have
            // an unload event for this chunk.
            if (chunk == nullptr) return true;

            // TODO(Matthew): These tasks are being added in the wrong event - this
            //                is the pre-voxel change event checking if the change may
            //                occur, in fact we should do this only after the voxel
            //                change has occurred.

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
    hmem::WeakHandle<ChunkGrid> self,
    ui32                        render_distance,
    ui32                        thread_count,
    ChunkTaskBuilder            build_load_or_generate_task,
    ChunkTaskBuilder            build_mesh_task,
    ChunkTaskBuilder*           build_navmesh_task /* = nullptr*/
) {
    m_self = self;

    m_render_distance           = render_distance;
    m_chunks_in_render_distance = render_distance * render_distance * render_distance;

    m_build_load_or_generate_task = build_load_or_generate_task;
    m_build_mesh_task             = build_mesh_task;
    if (build_navmesh_task) {
        m_build_navmesh_task = *build_navmesh_task;
    }

    m_thread_pool.init(thread_count);

    m_voxel_pager    = hmem::make_handle<ChunkVoxelPager>();
    m_instance_pager = hmem::make_handle<ChunkInstanceDataPager>();
    m_navmesh_pager  = hmem::make_handle<ai::ChunkNavmeshPager>();

    // TODO(Matthew): smarter setting of page size - maybe should be dependent on draw
    // distance. m_renderer.init(20, 2);
    m_renderer.init(5, 2);
}

void hvox::ChunkGrid::dispose() {
    m_thread_pool.dispose();

    m_renderer.dispose();
}

void hvox::ChunkGrid::update(FrameTime time) {
    for (auto chunk : m_chunks) {
        chunk.second->update(time);
    }

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
    auto it = m_chunks.find(chunk_position.id);
    if (it != m_chunks.end()) return false;

    hmem::Handle<Chunk> chunk = hmem::allocate_handle<Chunk>(m_chunk_allocator);
    chunk->position           = chunk_position;
    chunk->init(chunk, m_voxel_pager, m_instance_pager, m_navmesh_pager);

    chunk->on_load         += &handle_chunk_load;
    chunk->on_voxel_change += &handle_voxel_change;

    establish_chunk_neighbours(chunk);

    m_chunks[chunk_position.id] = chunk;

    m_renderer.add_chunk(chunk);

    return true;
}

bool hvox::ChunkGrid::load_chunk_at(ChunkGridPosition chunk_position) {
    preload_chunk_at(chunk_position);

    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return false;

    hmem::Handle<Chunk> chunk = (*it).second;

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

void hvox::ChunkGrid::establish_chunk_neighbours(hmem::Handle<Chunk> chunk) {
    ChunkGridPosition neighbour_position;

    // Update neighbours with info of new chunk.
    // LEFT
    neighbour_position   = chunk->position;
    neighbour_position.x -= 1;
    auto it              = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.left         = (*it).second;
        (*it).second->neighbours.one.right = chunk;
    } else {
        chunk->neighbours.one.left = hmem::WeakHandle<Chunk>();
    }

    // RIGHT
    neighbour_position   = chunk->position;
    neighbour_position.x += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.right       = (*it).second;
        (*it).second->neighbours.one.left = chunk;
    } else {
        chunk->neighbours.one.right = hmem::WeakHandle<Chunk>();
    }

    // TOP
    neighbour_position   = chunk->position;
    neighbour_position.y += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.top           = (*it).second;
        (*it).second->neighbours.one.bottom = chunk;
    } else {
        chunk->neighbours.one.top = hmem::WeakHandle<Chunk>();
    }

    // BOTTOM
    neighbour_position   = chunk->position;
    neighbour_position.y -= 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.bottom     = (*it).second;
        (*it).second->neighbours.one.top = chunk;
    } else {
        chunk->neighbours.one.bottom = hmem::WeakHandle<Chunk>();
    }

    // FRONT
    neighbour_position   = chunk->position;
    neighbour_position.z += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.front       = (*it).second;
        (*it).second->neighbours.one.back = chunk;
    } else {
        chunk->neighbours.one.front = hmem::WeakHandle<Chunk>();
    }

    // BACK
    neighbour_position   = chunk->position;
    neighbour_position.z -= 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.back         = (*it).second;
        (*it).second->neighbours.one.front = chunk;
    } else {
        chunk->neighbours.one.back = hmem::WeakHandle<Chunk>();
    }
}

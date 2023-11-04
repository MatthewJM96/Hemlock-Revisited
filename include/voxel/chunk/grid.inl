#include "stdafx.h"

#include "graphics/mesh.h"

#include "voxel/block.hpp"

template <hvox::ChunkDecorator... Decorations>
hvox::ChunkGrid<Decorations...>::ChunkGrid() :
    // TODO(Matthew): both of these are rather inefficient, we remesh literally every
    // block change, rather than
    //                once per update loop, likewise with chunk loads.
    // TODO(Matthew): as these are called in threadpool threads, we'd like them to
    //                be able to use the appropriate producer tokens.
    handle_chunk_load(Delegate<void(Sender)>{ [&](Sender sender) {
        hmem::WeakHandle<_Chunk> handle = sender.get_handle<_Chunk>();

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
            hmem::WeakHandle<_Chunk> handle = sender.get_handle<_Chunk>();

            auto chunk = handle.lock();
            // If chunk is nullptr, then there's no point
            // handling the block change as we will have
            // an unload event for this chunk.
            if (chunk == nullptr) return true;

            // TODO(Matthew): These tasks are being added in the wrong event - this
            //                is the pre-block change event checking if the change may
            //                occur, in fact we should do this only after the block
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

template <hvox::ChunkDecorator... Decorations>
void hvox::ChunkGrid<Decorations...>::init(
    hmem::WeakHandle<_ChunkGrid> self,
    ui32                         render_distance,
    ui32                         thread_count,
    _ChunkTaskBuilder            build_load_or_generate_task,
    _ChunkTaskBuilder            build_mesh_task,
    _ChunkTaskBuilder*           build_navmesh_task /* = nullptr*/
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

    m_block_pager    = hmem::make_handle<ChunkBlockPager>();
    m_instance_pager = hmem::make_handle<ChunkInstanceDataPager>();
    m_navmesh_pager  = hmem::make_handle<ai::ChunkNavmeshPager>();

    // TODO(Matthew): smarter setting of page size - maybe should be dependent on draw
    // distance. m_renderer.init(20, 2);
    m_renderer.init(5, 2);
}

template <hvox::ChunkDecorator... Decorations>
void hvox::ChunkGrid<Decorations...>::dispose() {
    m_thread_pool.dispose();

    m_renderer.dispose();
}

template <hvox::ChunkDecorator... Decorations>
void hvox::ChunkGrid<Decorations...>::update(FrameTime time) {
    for (auto chunk : m_chunks) {
        chunk.second->update(time);
    }

    m_renderer.update(time);
}

template <hvox::ChunkDecorator... Decorations>
void hvox::ChunkGrid<Decorations...>::draw(FrameTime time) {
    m_renderer.draw(time);
}

template <hvox::ChunkDecorator... Decorations>
void hvox::ChunkGrid<Decorations...>::set_render_distance(ui32 render_distance) {
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

template <hvox::ChunkDecorator... Decorations>
bool hvox::ChunkGrid<Decorations...>::load_chunks(
    ChunkGridPosition* chunk_positions, ui32 chunk_count
) {
    bool any_chunk_failed = false;

    for (ui32 i = 0; i < chunk_count; ++i) {
        any_chunk_failed = any_chunk_failed || load_chunk_at(chunk_positions[i]);
    }

    return any_chunk_failed;
}

template <hvox::ChunkDecorator... Decorations>
bool hvox::ChunkGrid<Decorations...>::preload_chunk_at(ChunkGridPosition chunk_position
) {
    auto it = m_chunks.find(chunk_position.id);
    if (it != m_chunks.end()) return false;

    hmem::Handle<_Chunk> chunk = hmem::allocate_handle<_Chunk>(m_chunk_allocator);
    chunk->position            = chunk_position;
    chunk->init(chunk, m_block_pager, m_instance_pager, m_navmesh_pager);

    chunk->on_load         += &handle_chunk_load;
    chunk->on_block_change += &handle_block_change;

    establish_chunk_neighbours(chunk);

    m_chunks[chunk_position.id] = chunk;

    m_renderer.add_chunk(chunk);

    return true;
}

template <hvox::ChunkDecorator... Decorations>
bool hvox::ChunkGrid<Decorations...>::load_chunk_at(ChunkGridPosition chunk_position) {
    preload_chunk_at(chunk_position);

    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return false;

    hmem::Handle<_Chunk> chunk = (*it).second;

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

template <hvox::ChunkDecorator... Decorations>
bool hvox::ChunkGrid<Decorations...>::unload_chunk_at(
    ChunkGridPosition chunk_position, hmem::WeakHandle<_Chunk>* handle /*= nullptr*/
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

template <hvox::ChunkDecorator... Decorations>
hmem::Handle<hvox::Chunk<Decorations...>>
hvox::ChunkGrid<Decorations...>::chunk(ChunkID id) {
    auto it = m_chunks.find(id);

    if (it == m_chunks.end()) return nullptr;

    return it->second;
}

template <hvox::ChunkDecorator... Decorations>
void hvox::ChunkGrid<Decorations...>::establish_chunk_neighbours(
    hmem::Handle<_Chunk> chunk
) {
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
        chunk->neighbours.one.left = hmem::WeakHandle<_Chunk>();
    }

    // RIGHT
    neighbour_position   = chunk->position;
    neighbour_position.x += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.right       = (*it).second;
        (*it).second->neighbours.one.left = chunk;
    } else {
        chunk->neighbours.one.right = hmem::WeakHandle<_Chunk>();
    }

    // TOP
    neighbour_position   = chunk->position;
    neighbour_position.y += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.top           = (*it).second;
        (*it).second->neighbours.one.bottom = chunk;
    } else {
        chunk->neighbours.one.top = hmem::WeakHandle<_Chunk>();
    }

    // BOTTOM
    neighbour_position   = chunk->position;
    neighbour_position.y -= 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.bottom     = (*it).second;
        (*it).second->neighbours.one.top = chunk;
    } else {
        chunk->neighbours.one.bottom = hmem::WeakHandle<_Chunk>();
    }

    // FRONT
    neighbour_position   = chunk->position;
    neighbour_position.z += 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.front       = (*it).second;
        (*it).second->neighbours.one.back = chunk;
    } else {
        chunk->neighbours.one.front = hmem::WeakHandle<_Chunk>();
    }

    // BACK
    neighbour_position   = chunk->position;
    neighbour_position.z -= 1;
    it                   = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.back         = (*it).second;
        (*it).second->neighbours.one.front = chunk;
    } else {
        chunk->neighbours.one.back = hmem::WeakHandle<_Chunk>();
    }
}

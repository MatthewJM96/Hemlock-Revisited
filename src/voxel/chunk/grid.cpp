#include "stdafx.h"

#include "graphics/mesh.h"

#include "voxel/block.hpp"
#include "voxel/chunk/grid.h"

void hvox::ChunkLoadTask::init(hmem::Handle<Chunk> chunk, ChunkGrid* chunk_grid) {
    m_chunk      = chunk;
    m_chunk_grid = chunk_grid;
}

void hvox::ChunkGrid::init(                       ui32 thread_count,
                            thread::ThreadWorkflowDAG* chunk_load_dag,
                              ChunkLoadTaskListBuilder chunk_load_task_list_builder )
{
    build_load_tasks = chunk_load_task_list_builder;

    m_chunk_load_thread_pool.init(thread_count);
    m_chunk_load_workflow.init(chunk_load_dag, &m_chunk_load_thread_pool);

    // TODO(Matthew): smarter setting of page size - maybe should be dependent on draw distance.
    m_renderer.init(20, 2);
}

void hvox::ChunkGrid::dispose() {
    m_chunk_load_thread_pool.dispose();
    m_chunk_load_workflow.dispose();
}

void hvox::ChunkGrid::update(TimeData time) {
    // Update chunks, removing those marked for unload that
    // are done with pending tasks.
    for (auto it = m_chunks.begin(); it != m_chunks.end();) {
        // TODO(Matthew): we shouldn't do this explicitly with unload.
        if ((*it).second->unload.load(std::memory_order_acquire)) {
            auto [exists, not_pending] =
                query_chunk_exact_pending_task(
                    (*it).second,
                    ChunkLoadTaskKind::NONE
                );

            // If we are pending any task, don't unload
            // the task just yet.
            if (!not_pending) {
                ++it;
                continue;
            }

            (*it).second->on_unload();

            (*it).second->dispose();
            delete (*it).second;

            it = m_chunks.erase(it);
        } else {
            (*it).second->update(time);
            ++it;
        }
    }

    m_renderer.update(time);
}

void hvox::ChunkGrid::draw(TimeData time) {
    m_renderer.draw(time);
}

bool hvox::ChunkGrid::load_from_scratch_chunks(ChunkGridPosition* chunk_positions, ui32 chunk_count) {
    bool any_chunk_failed = false;

    for (ui32 i = 0; i < chunk_count; ++i) {
        any_chunk_failed = any_chunk_failed || preload_chunk_at(chunk_positions[i]);
    }

    for (ui32 i = 0; i < chunk_count; ++i) {
        any_chunk_failed = any_chunk_failed || load_chunk_at(chunk_positions[i]);
    }

    return any_chunk_failed;
}

bool hvox::ChunkGrid::preload_chunk_at(ChunkGridPosition chunk_position) {
    auto it = m_chunks.find(chunk_position.id);
    if (it != m_chunks.end()) return false;

    hmem::Handle<Chunk> chunk = m_chunk_allocator.allocate();
    chunk->position = chunk_position;
    chunk->init();

    establish_chunk_neighbours(chunk);

    m_chunks[chunk_position.id] = chunk;

    m_renderer.add_chunk(chunk);

    return true;
}

bool hvox::ChunkGrid::load_chunk_at(ChunkGridPosition chunk_position) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return false;

    hmem::Handle<Chunk> chunk = (*it).second;

    // Note the order of these checks is probably important.
    // I have guessed for now but expect if we queried pending
    // after actual state we could encounter race conditions
    // where the chunk was generated just as we were going
    // from the first to the second query.
    //   In this order, we should get no race condition
    //   as submitting generation tasks should be only
    //   done from the main thread. That said, we have
    //   to be careful that we also appropriately update
    //   these states in the tasks.

    // If chunk is in the process of being generated, we don't
    // need to add it to the queue again.
    auto [ _1, chunk_pending_generation ] =
            query_chunk_pending_task(chunk, ChunkLoadTaskKind::GENERATION);
    if (chunk_pending_generation)
        return false;

    // If chunk is already generated, we don't need to try
    // to generate it again.
    auto [ _2, chunk_already_generated ] =
            query_chunk_state(chunk, ChunkState::GENERATED);
    if (chunk_already_generated)
        return false;

    // If chunk is not yet preloaded, we can't try to
    // generate it.
    auto [ _3, chunk_preloaded ] =
            query_chunk_state(chunk, ChunkState::PRELOADED);
    if (!chunk_preloaded)
        return false;

    auto tasks = build_load_tasks(chunk, this);

    m_chunk_load_workflow.run(tasks);
    chunk->pending_task.store(ChunkLoadTaskKind::GENERATION, std::memory_order_release);

    return true;
}

bool hvox::ChunkGrid::load_from_scratch_chunk_at(ChunkGridPosition chunk_position) {
    preload_chunk_at(chunk_position);

    return load_chunk_at(chunk_position);
}

bool hvox::ChunkGrid::unload_chunk_at(ChunkGridPosition chunk_position) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return false;

    (*it).second->on_unload();

    (*it).second.release();

    m_chunks.erase(it);

    return true;
}

void hvox::ChunkGrid::establish_chunk_neighbours(hmem::Handle<Chunk> chunk) {
    ChunkGridPosition neighbour_position;

    // Update neighbours with info of new chunk.
    // LEFT
    neighbour_position = chunk->position;
    neighbour_position.x -= 1;
    auto it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.left = (*it).second;
        (*it).second->neighbours.right = chunk;
    } else {
        chunk->neighbours.left = hmem::Handle<Chunk>();
    }

    // RIGHT
    neighbour_position = chunk->position;
    neighbour_position.x += 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.right = (*it).second;
        (*it).second->neighbours.left = chunk;
    } else {
        chunk->neighbours.right = hmem::Handle<Chunk>();
    }

    // TOP
    neighbour_position = chunk->position;
    neighbour_position.y += 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.top = (*it).second;
        (*it).second->neighbours.bottom = chunk;
    } else {
        chunk->neighbours.top = hmem::Handle<Chunk>();
    }

    // BOTTOM
    neighbour_position = chunk->position;
    neighbour_position.y -= 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.bottom = (*it).second;
        (*it).second->neighbours.top = chunk;
    } else {
        chunk->neighbours.bottom = hmem::Handle<Chunk>();
    }

    // FRONT
    neighbour_position = chunk->position;
    neighbour_position.z -= 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.front = (*it).second;
        (*it).second->neighbours.back = chunk;
    } else {
        chunk->neighbours.front = hmem::Handle<Chunk>();
    }

    // BACK
    neighbour_position = chunk->position;
    neighbour_position.z += 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.back = (*it).second;
        (*it).second->neighbours.front = chunk;
    } else {
        chunk->neighbours.back = hmem::Handle<Chunk>();
    }
}

hvox::QueriedChunkState hvox::ChunkGrid::query_chunk_state(ChunkGridPosition chunk_position, ChunkState required_minimum_state) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return {false, false};

    return query_chunk_state((*it).second, required_minimum_state);
}

hvox::QueriedChunkState hvox::ChunkGrid::query_chunk_state(hmem::Handle<Chunk> chunk, ChunkState required_minimum_state) {
    if (chunk == nullptr) return {false, false};

    ChunkState actual_state = chunk->state.load(std::memory_order_acquire);

    return {true, actual_state >= required_minimum_state};
}

hvox::QueriedChunkPendingTask hvox::ChunkGrid::query_chunk_pending_task(ChunkGridPosition chunk_position, ChunkLoadTaskKind required_minimum_pending_task) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return {false, false};

    return query_chunk_pending_task((*it).second, required_minimum_pending_task);
}

hvox::QueriedChunkPendingTask hvox::ChunkGrid::query_chunk_pending_task(hmem::Handle<Chunk> chunk, ChunkLoadTaskKind required_minimum_pending_task) {
    if (chunk == nullptr) return {false, false};

    ChunkLoadTaskKind actual_pending_task = chunk->pending_task.load(std::memory_order_acquire);

    return {true, actual_pending_task >= required_minimum_pending_task};
}

hvox::QueriedChunkState hvox::ChunkGrid::query_all_neighbour_states(ChunkGridPosition chunk_position, ChunkState required_minimum_state) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return {false, false};

    return query_all_neighbour_states((*it).second, required_minimum_state);
}

hvox::QueriedChunkState hvox::ChunkGrid::query_all_neighbour_states(hmem::Handle<Chunk> chunk, ChunkState required_minimum_state) {
    if (chunk == nullptr) return {false, false};

    bool all_neighbours_satisfy_constraint = true;
    for (ui32 i = 0; i < 8; ++i) {
        if (chunk->neighbours.neighbours[i] == nullptr) continue;

        ChunkState actual_state = chunk->neighbours.neighbours[i]->state.load(std::memory_order_acquire);

        all_neighbours_satisfy_constraint = all_neighbours_satisfy_constraint && (actual_state >= required_minimum_state);
    }

    return {true, all_neighbours_satisfy_constraint};
}

hvox::QueriedChunkState hvox::ChunkGrid::query_chunk_exact_state(ChunkGridPosition chunk_position, ChunkState required_state) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return {false, false};

    return query_chunk_exact_state((*it).second, required_state);
}

hvox::QueriedChunkState hvox::ChunkGrid::query_chunk_exact_state(hmem::Handle<Chunk> chunk, ChunkState required_state) {
    if (chunk == nullptr) return {false, false};

    ChunkState actual_state = chunk->state.load(std::memory_order_acquire);

    return {true, actual_state == required_state};
}

hvox::QueriedChunkPendingTask hvox::ChunkGrid::query_chunk_exact_pending_task(ChunkGridPosition chunk_position, ChunkLoadTaskKind required_pending_task) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return {false, false};

    return query_chunk_exact_pending_task((*it).second, required_pending_task);
}

hvox::QueriedChunkPendingTask hvox::ChunkGrid::query_chunk_exact_pending_task(hmem::Handle<Chunk> chunk, ChunkLoadTaskKind required_pending_task) {
    if (chunk == nullptr) return {false, false};

    ChunkLoadTaskKind actual_pending_task = chunk->pending_task.load(std::memory_order_acquire);

    return {true, actual_pending_task == required_pending_task};
}

hvox::QueriedChunkState hvox::ChunkGrid::query_all_neighbour_exact_states(ChunkGridPosition chunk_position, ChunkState required_state) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return {false, false};

    return query_all_neighbour_exact_states((*it).second, required_state);
}

hvox::QueriedChunkState hvox::ChunkGrid::query_all_neighbour_exact_states(hmem::Handle<Chunk> chunk, ChunkState required_state) {
    if (chunk == nullptr) return {false, false};

    bool all_neighbours_satisfy_constraint = true;
    for (ui32 i = 0; i < 8; ++i) {
        if (chunk->neighbours.neighbours[i] == nullptr) continue;

        ChunkState actual_state = chunk->neighbours.neighbours[i]->state.load(std::memory_order_acquire);

        all_neighbours_satisfy_constraint = all_neighbours_satisfy_constraint && (actual_state == required_state);
    }

    return {true, all_neighbours_satisfy_constraint};
}

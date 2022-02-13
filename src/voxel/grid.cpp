#include "stdafx.h"

#include "voxel/generator.h"

#include "voxel/grid.h"

void hvox::ChunkLoadTask::init(Chunk* chunk, ChunkGrid* chunk_grid) {
    m_chunk      = chunk;
    m_chunk_grid = chunk_grid;
}

void hvox::ChunkGrid::init(ui32 thread_count) {
    m_gen_threads.init(thread_count);
}

void hvox::ChunkGrid::dispose() {
    m_gen_threads.dispose();
}

void hvox::ChunkGrid::update(TimeData time) {
    for (auto& chunk : m_chunks) {
        chunk.second->update(time);
    }
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

    Chunk* chunk = new Chunk();
    chunk->position = chunk_position;
    chunk->init();

    // TODO(Matthew): Right now we're getting a segfult on glDrawArrays with this enabled.
    //                Inevitably it must be something to do with the buffers being uploaded
    //                to the GPU, but all the basics are fine: VAOs are set up, vertex
    //                buffer looks fine on a quick eyeball, and vertex count corresponds
    //                to expected cases of either being a corner, face, or edge chunk.
    //                    The VAO on which the segfault occurs also seems to be random,
    //                    it feels likely this is useful information but right now
    //                    is just confusing. We know at least it isn't the first
    //                    encountered by glDrawArrays as we see some chunks appear before
    //                    the program crashes.
    //                        Check if any of the three chunk location cases renders fine?
    //                            Corners have so far not shown up but that could just be odds.
    establish_chunk_neighbours(chunk);

    m_chunks[chunk_position.id] = chunk;

    return true;
}

bool hvox::ChunkGrid::load_chunk_at(ChunkGridPosition chunk_position) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return false;

    Chunk* chunk = (*it).second;

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

    auto task = new ChunkGenerationTask();
    task->init(chunk, this);
    m_gen_threads.add_task({ task, true });
    chunk->pending_task.store(ChunkLoadTaskKind::GENERATION, std::memory_order_release);

    return true;
}

bool hvox::ChunkGrid::unload_chunk_at(ChunkGridPosition chunk_position) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return false;

    // TODO(Matthew): Should check for pending tasks, and wait on them before unloading.
    // TODO(Matthew): Maybe we want to even add an unload task so things can hook into this.
    //                    But then tbh we probably want to just use an event for this.

    (*it).second->dispose();
    delete (*it).second;

    m_chunks.erase(it);

    return true;
}

void hvox::ChunkGrid::establish_chunk_neighbours(Chunk* chunk) {
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
        chunk->neighbours.left = nullptr;
    }

    // RIGHT
    neighbour_position = chunk->position;
    neighbour_position.x += 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.right = (*it).second;
        (*it).second->neighbours.left = chunk;
    } else {
        chunk->neighbours.right = nullptr;
    }

    // TOP
    neighbour_position = chunk->position;
    neighbour_position.y += 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.top = (*it).second;
        (*it).second->neighbours.bottom = chunk;
    } else {
        chunk->neighbours.top = nullptr;
    }

    // BOTTOM
    neighbour_position = chunk->position;
    neighbour_position.y -= 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.bottom = (*it).second;
        (*it).second->neighbours.top = chunk;
    } else {
        chunk->neighbours.bottom = nullptr;
    }

    // FRONT
    neighbour_position = chunk->position;
    neighbour_position.z -= 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.front = (*it).second;
        (*it).second->neighbours.back = chunk;
    } else {
        chunk->neighbours.front = nullptr;
    }

    // BACK
    neighbour_position = chunk->position;
    neighbour_position.z += 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.back = (*it).second;
        (*it).second->neighbours.front = chunk;
    } else {
        chunk->neighbours.back = nullptr;
    }
}

hvox::QueriedChunkState hvox::ChunkGrid::query_chunk_state(ChunkGridPosition chunk_position, ChunkState required_minimum_state) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return {false, false};

    return query_chunk_state((*it).second, required_minimum_state);
}

hvox::QueriedChunkState hvox::ChunkGrid::query_chunk_state(Chunk* chunk, ChunkState required_minimum_state) {
    if (chunk == nullptr) return {false, false};

    ChunkState actual_state = chunk->state.load(std::memory_order_acquire);

    return {true, actual_state >= required_minimum_state};
}

hvox::QueriedChunkPendingTask hvox::ChunkGrid::query_chunk_pending_task(ChunkGridPosition chunk_position, ChunkLoadTaskKind required_minimum_pending_task) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return {false, false};

    return query_chunk_pending_task((*it).second, required_minimum_pending_task);
}

hvox::QueriedChunkPendingTask hvox::ChunkGrid::query_chunk_pending_task(Chunk* chunk, ChunkLoadTaskKind required_minimum_pending_task) {
    if (chunk == nullptr) return {false, false};

    ChunkLoadTaskKind actual_pending_task = chunk->pending_task.load(std::memory_order_acquire);

    return {true, actual_pending_task >= required_minimum_pending_task};
}

hvox::QueriedChunkState hvox::ChunkGrid::query_all_neighbour_states(ChunkGridPosition chunk_position, ChunkState required_minimum_state) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return {false, false};

    return query_all_neighbour_states((*it).second, required_minimum_state);
}

hvox::QueriedChunkState hvox::ChunkGrid::query_all_neighbour_states(Chunk* chunk, ChunkState required_minimum_state) {
    if (chunk == nullptr) return {false, false};

    bool all_neighbours_satisfy_constraint = true;
    for (ui32 i = 0; i < 8; ++i) {
        if (chunk->neighbours.neighbours[i] == nullptr) continue;

        ChunkState actual_state = chunk->neighbours.neighbours[i]->state.load(std::memory_order_acquire);

        all_neighbours_satisfy_constraint = all_neighbours_satisfy_constraint && (actual_state >= required_minimum_state);
    }

    return {true, all_neighbours_satisfy_constraint};
}

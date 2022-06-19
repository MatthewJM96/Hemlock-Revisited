#include "stdafx.h"

#include "graphics/mesh.h"

#include "voxel/block.hpp"
#include "voxel/chunk/grid.h"

void hvox::ChunkLoadTask::init(hmem::WeakHandle<Chunk> chunk, hmem::WeakHandle<ChunkGrid> chunk_grid) {
    m_chunk      = chunk;
    m_chunk_grid = chunk_grid;
}

void hvox::ChunkGrid::init( hmem::WeakHandle<ChunkGrid> self,
                                                   ui32 thread_count,
                             thread::ThreadWorkflowDAG* chunk_load_dag,
                               ChunkLoadTaskListBuilder chunk_load_task_list_builder )
{
    m_self = self;

    build_load_tasks = chunk_load_task_list_builder;

    m_chunk_load_thread_pool.init(thread_count);
    m_chunk_load_workflow.init(chunk_load_dag, &m_chunk_load_thread_pool);

    // TODO(Matthew): smarter setting of page size - maybe should be dependent on draw distance.
    // m_renderer.init(20, 2);
    m_renderer.init(5, 2);


    // TODO(Matthew): MOVE IT
    glCreateVertexArrays(1, &m_grid_vao);

    glCreateBuffers(1, &m_grid_vbo);
    glNamedBufferData(
        m_grid_vbo,
        sizeof(f32v3) * 2 * 12 * (100 * 100 * 100), // 2 points per line, 12 lines per chunk, a whole lotta chunks.
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexArrayVertexBuffer(m_grid_vao, 0, m_grid_vbo, 0, sizeof(f32v3));

    glEnableVertexArrayAttrib(m_grid_vao, 0);
    glVertexArrayAttribFormat(m_grid_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_grid_vao, 0, 0);

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

void hvox::ChunkGrid::draw_grid() {
    // TODO(Matthew): deduplicate lines.

    std::vector<f32v3> lines;
    for (auto& [id, chunk] : m_chunks) {
        // bottom back left - top back left
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0)));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{0, CHUNK_SIZE, 0}));

        // bottom back left - bottom front left
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0)));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{0, 0, CHUNK_SIZE}));

        // bottom back left - bottom back right
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0)));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, 0, 0}));

        // top front right - top front left
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE}));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{0, CHUNK_SIZE, CHUNK_SIZE}));

        // top front right - top back right
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE}));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, CHUNK_SIZE, 0}));

        // top front right - bottom front right
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE}));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, 0, CHUNK_SIZE}));

        // top back left - top front left
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{0, CHUNK_SIZE, 0}));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{0, CHUNK_SIZE, CHUNK_SIZE}));

        // top front left - bottom front left
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{0, CHUNK_SIZE, CHUNK_SIZE}));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{0, 0, CHUNK_SIZE}));

        // top back left - top back right
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{0, CHUNK_SIZE, 0}));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, CHUNK_SIZE, 0}));

        // bottom front right - bottom front left
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, 0, CHUNK_SIZE}));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{0, 0, CHUNK_SIZE}));

        // bottom front right - bottom back right
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, 0, CHUNK_SIZE}));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, 0, 0}));

        // bottom back right - top back right
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, 0, 0}));
        lines.emplace_back(f32v3(block_world_position(chunk->position, 0) + i32v3{CHUNK_SIZE, CHUNK_SIZE, 0}));
    }

    glNamedBufferSubData(
        m_grid_vbo, 0, lines.size() * sizeof(f32v3),
        reinterpret_cast<void*>(&lines[0])
    );

    glBindVertexArray(m_grid_vao);
    glDrawArrays(GL_LINES, 0, lines.size());
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

    hmem::Handle<Chunk> chunk = hmem::allocate_handle<Chunk>(m_chunk_allocator);
    chunk->position = chunk_position;
    chunk->init(chunk);

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

    m_chunk_load_workflow.run(
        build_load_tasks(chunk, m_self)
    );
    chunk->pending_task.store(ChunkLoadTaskKind::GENERATION, std::memory_order_release);

    return true;
}

bool hvox::ChunkGrid::load_from_scratch_chunk_at(ChunkGridPosition chunk_position) {
    preload_chunk_at(chunk_position);

    return load_chunk_at(chunk_position);
}

bool hvox::ChunkGrid::unload_chunk_at(ChunkGridPosition chunk_position, hmem::WeakHandle<Chunk>* handle /*= nullptr*/) {
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

void hvox::ChunkGrid::establish_chunk_neighbours(hmem::Handle<Chunk> chunk) {
    ChunkGridPosition neighbour_position;

    // Update neighbours with info of new chunk.
    // LEFT
    neighbour_position = chunk->position;
    neighbour_position.x -= 1;
    auto it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.left = (*it).second;
        (*it).second->neighbours.one.right = chunk;
    } else {
        chunk->neighbours.one.left = hmem::WeakHandle<Chunk>();
    }

    // RIGHT
    neighbour_position = chunk->position;
    neighbour_position.x += 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.right = (*it).second;
        (*it).second->neighbours.one.left = chunk;
    } else {
        chunk->neighbours.one.right = hmem::WeakHandle<Chunk>();
    }

    // TOP
    neighbour_position = chunk->position;
    neighbour_position.y += 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.top = (*it).second;
        (*it).second->neighbours.one.bottom = chunk;
    } else {
        chunk->neighbours.one.top = hmem::WeakHandle<Chunk>();
    }

    // BOTTOM
    neighbour_position = chunk->position;
    neighbour_position.y -= 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.bottom = (*it).second;
        (*it).second->neighbours.one.top = chunk;
    } else {
        chunk->neighbours.one.bottom = hmem::WeakHandle<Chunk>();
    }

    // FRONT
    neighbour_position = chunk->position;
    neighbour_position.z -= 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.front = (*it).second;
        (*it).second->neighbours.one.back = chunk;
    } else {
        chunk->neighbours.one.front = hmem::WeakHandle<Chunk>();
    }

    // BACK
    neighbour_position = chunk->position;
    neighbour_position.z += 1;
    it = m_chunks.find(neighbour_position.id);
    if (it != m_chunks.end()) {
        chunk->neighbours.one.back = (*it).second;
        (*it).second->neighbours.one.front = chunk;
    } else {
        chunk->neighbours.one.back = hmem::WeakHandle<Chunk>();
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
        auto neighbour = chunk->neighbours.all[i].lock();
        if (neighbour == nullptr) continue;

        ChunkState actual_state = neighbour->state.load(std::memory_order_acquire);

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
        auto neighbour = chunk->neighbours.all[i].lock();
        if (neighbour == nullptr) continue;

        ChunkState actual_state = neighbour->state.load(std::memory_order_acquire);

        all_neighbours_satisfy_constraint = all_neighbours_satisfy_constraint && (actual_state == required_state);
    }

    return {true, all_neighbours_satisfy_constraint};
}

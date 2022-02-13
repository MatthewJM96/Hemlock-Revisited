#include "stdafx.h"

#include "voxel/loader.h"

#include "voxel/grid.h"

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

bool hvox::ChunkGrid::load_chunk_at(ChunkGridPosition chunk_position) {
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

    auto task = new ChunkLoadTask();
    task->init(chunk);
    m_gen_threads.add_task({ task, true });

    return true;
}

bool hvox::ChunkGrid::unload_chunk_at(ChunkGridPosition chunk_position) {
    auto it = m_chunks.find(chunk_position.id);
    if (it == m_chunks.end()) return false;

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
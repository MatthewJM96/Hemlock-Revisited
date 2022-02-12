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

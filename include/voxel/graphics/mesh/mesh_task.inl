template <hvox::ChunkMeshStrategy MeshStrategy>
void hvox::ChunkMeshTask<
    MeshStrategy>::execute(ChunkLoadThreadState* state, ChunkTaskQueue* task_queue) {
    auto chunk_grid = m_chunk_grid.lock();
    if (chunk_grid == nullptr) return;

    auto chunk = m_chunk.lock();
    if (chunk == nullptr) return;

    const MeshStrategy mesh{};

    if (!mesh.can_run(chunk_grid, chunk)) {
        // Put copy of this mesh task back onto the load task queue.
        ChunkMeshTask<MeshStrategy>* mesh_task
            = new ChunkMeshTask<MeshStrategy>();
        mesh_task->set_state(m_chunk, m_chunk_grid);
        task_queue->enqueue(state->producer_token, { mesh_task, true });
        return;
    }

    chunk->meshing.store(ChunkState::ACTIVE, std::memory_order_release);

    mesh(chunk_grid, chunk);

    chunk->meshing.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk->on_mesh_change();
}

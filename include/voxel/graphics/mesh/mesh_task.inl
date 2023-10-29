template <hvox::ChunkMeshStrategy MeshStrategy>
void hvox::ChunkMeshTask<MeshStrategy>::execute(
    ChunkLoadThreadState* state, ChunkTaskQueue* task_queue
) {
    auto chunk_grid = m_chunk_grid.lock();
    if (chunk_grid == nullptr) return;

    if (!chunk_grid->registry()->valid(m_chunk)) return;

    auto& chunk_mesh = chunk_grid->registry()->get<ChunkMeshComponent>(m_chunk);

    const MeshStrategy mesh{};

    if (!mesh.can_run(chunk_grid, m_chunk)) {
        // Put copy of this mesh task back onto the load task queue.
        ChunkMeshTask<MeshStrategy>* mesh_task = new ChunkMeshTask<MeshStrategy>();
        mesh_task->set_state(m_chunk, m_chunk_grid);
        task_queue->enqueue(state->producer_token, { mesh_task, true });
        return;
    }

    chunk_mesh.meshing.store(ChunkState::ACTIVE, std::memory_order_release);

    mesh(chunk_grid, m_chunk);

    chunk_mesh.meshing.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk_mesh.on_mesh_change();
}

template <hvox::CelluarAutomataStrategy CaStrategy>
void hvox::CellularAutomataTask<CaStrategy>::execute(
    ChunkTaskThreadState* state, ChunkTaskQueue* task_queue
) {
    auto chunk_grid = m_chunk_grid.lock();
    if (chunk_grid == nullptr) return;

    auto chunk = m_chunk.lock();
    if (chunk == nullptr) return;

    const CaStrategy ca_strategy{};

    if (!ca_strategy.can_run(chunk_grid, chunk)) {
        // Put copy of this mesh task back onto the load task queue.
        CellularAutomataTask<CaStrategy>* ca_task
            = new CellularAutomataTask<CaStrategy>();
        ca_task->set_chunk_state(m_chunk, m_chunk_grid);
        task_queue->enqueue(state->producer_token, { ca_task, true });
        return;
    }

    // TODO(Matthew): we will either be here handling an entire chunk as a task, or else
    //                a single block index within a chunk as a task. The latter can't
    //                really be done with determinism. Likely we pursue the former
    //                with a (double?) buffer storing indices of changed blocks
    //                (resizing periodically to keep down memory usage). Apply sort
    //                over this before iteration.

    ca_strategy(chunk_grid, chunk /*, block_index*/);
}

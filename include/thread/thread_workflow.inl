template <hthread::IsThreadState ThreadState>
void hthread::IThreadWorkflowTask<ThreadState>::dispose() {
    m_tasks                  = {};
    m_dag                    = nullptr;
    m_task_completion_states = {};
}

template <hthread::IsThreadState ThreadState>
void hthread::IThreadWorkflowTask<ThreadState>::set_workflow_metadata(
    ThreadWorkflowTasksView<ThreadState> tasks,
    ThreadWorkflowTaskID                 task_idx,
    ThreadWorkflowDAG*                   dag,
    ThreadWorkflowTaskCompletionView     task_completion_states
) {
    m_tasks                  = tasks;
    m_task_idx               = task_idx;
    m_dag                    = dag;
    m_task_completion_states = task_completion_states;
}

template <hthread::IsThreadState ThreadState>
void hthread::IThreadWorkflowTask<ThreadState>::execute(
    typename Thread<ThreadState>::State* state, BasicTaskQueue<ThreadState>* task_queue
) {
    if (run_task(state, task_queue) && m_dag) {
        auto [start, last] = m_dag->graph.equal_range(m_task_idx);
        for (; start != last; ++start) {
            auto next_task_idx = (*start).second;

            // fetch_add returns value before add!
            ui32 into_completed
                = m_task_completion_states.completion_states.get()[next_task_idx]
                      .fetch_add(1)
                  + 1;
            if (into_completed == m_dag->into_counts[next_task_idx]) {
                m_tasks.tasks.get()[next_task_idx].task->set_workflow_metadata(
                    m_tasks, next_task_idx, m_dag, m_task_completion_states
                );
                task_queue->enqueue(
                    state->producer_token,
                    { m_tasks.tasks.get()[next_task_idx].task,
                      m_tasks.tasks.get()[next_task_idx].should_delete }
                );
            }
        }
    }
}

template <hthread::IsThreadState ThreadState>
void hthread::ThreadWorkflow<ThreadState>::init(
    ThreadWorkflowDAG* dag, ThreadPool<ThreadState>* thread_pool
) {
    assert(dag != nullptr);
    assert(thread_pool != nullptr);

    m_dag         = dag;
    m_thread_pool = thread_pool;
}

template <hthread::IsThreadState ThreadState>
void hthread::ThreadWorkflow<ThreadState>::dispose() {
    m_dag         = nullptr;
    m_thread_pool = nullptr;
}

template <hthread::IsThreadState ThreadState>
void hthread::ThreadWorkflow<ThreadState>::run(
    ThreadWorkflowTasksView<ThreadState> tasks
) {
    ThreadWorkflowTaskCompletionView task_completion_states;
    task_completion_states.completion_states
        = hmem::Handle<ThreadWorkflowTaskCompletion[]>(
            new ThreadWorkflowTaskCompletion[m_dag->task_count]
        );
    task_completion_states.count = m_dag->task_count;

    // TODO(Matthew): tasks is being copied into metadata of each task in the
    // workflow. Can be done better.
    for (auto entry_task : m_dag->entry_tasks) {
        tasks.tasks.get()[entry_task].task->set_workflow_metadata(
            tasks, entry_task, m_dag, task_completion_states
        );
        m_thread_pool->add_task({ tasks.tasks.get()[entry_task].task,
                                  tasks.tasks.get()[entry_task].should_delete });
    }
}

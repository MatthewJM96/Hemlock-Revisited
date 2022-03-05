template <hthread::InterruptibleState ThreadState>
void hthread::IThreadWorkflowTask<ThreadState>::dispose() {
    m_tasks                  = {nullptr, 0};
    m_dag                    = nullptr;
    m_task_completion_states = nullptr;
}

template <hthread::InterruptibleState ThreadState>
void hthread::IThreadWorkflowTask<ThreadState>::set_workflow_metadata( ThreadWorkflowTaskList<ThreadState> tasks,
                                                                                      ThreadWorkflowTaskID task_idx,
                                                                                        ThreadWorkflowDAG* dag,
                                                                          ThreadWorkflowTaskCompletionView task_completion_states )
{
    m_tasks                  = tasks;
    m_task_idx               = task_idx;
    m_dag                    = dag;
    m_task_completion_states = task_completion_states;
}

template <hthread::InterruptibleState ThreadState>
void hthread::IThreadWorkflowTask<ThreadState>::execute(typename Thread<ThreadState>::State* state, TaskQueue<ThreadState>* task_queue) {
    if (run_task(state, task_queue)) {
        auto [start, last] = m_dag->graph.equal_range(m_task_idx);
        for (; start != last; ++start) {
            auto next_task_idx = (*start).second;

            ui32 into_completed = m_task_completion_states[next_task_idx].fetch_add(1);
            if (into_completed == m_dag->into_counts[next_task_idx]) {
                m_tasks[next_task_idx].task->set_workflow_metadata(m_tasks, next_task_idx, m_dag, m_task_completion_states);
                task_queue->enqueue(state->producer_token, m_tasks[next_task_idx]);
            }
        }
    }
}

template <hthread::InterruptibleState ThreadState>
void hthread::ThreadWorkflow<ThreadState>::init(ThreadWorkflowDAG* dag, ThreadPool<ThreadState>* thread_pool) {
    assert(         dag != nullptr );
    assert( thread_pool != nullptr );

    m_dag         = dag;
    m_thread_pool = thread_pool;
}

template <hthread::InterruptibleState ThreadState>
void hthread::ThreadWorkflow<ThreadState>::dispose() {
    m_dag         = nullptr;
    m_thread_pool = nullptr;
}

template <hthread::InterruptibleState ThreadState>
void hthread::ThreadWorkflow<ThreadState>::run(ThreadWorkflowTaskList<ThreadState> tasks) {
    ThreadWorkflowTaskCompletion* task_completion_states = new ThreadWorkflowTaskCompletion[m_dag->into_counts.size()]{};

    for (auto entry_task : m_dag->entry_tasks) {
        tasks[entry_task].task
            ->set_workflow_metadata(
                tasks,
                entry_task,
                m_dag,
                { task_completion_states, m_dag->into_counts.size() }
        );
        m_thread_pool->add_task(tasks[entry_task]);
    }

    // cleanup_run(tasks);
}

template <hthread::InterruptibleState ThreadState>
void hthread::ThreadWorkflow<ThreadState>::cleanup_run(ThreadWorkflowTaskList<ThreadState> tasks) {
    for (auto task : tasks) {
        if (task.should_delete)
            delete task.task;
    }
}

hthread::IThreadWorkflowTask::~IThreadWorkflowTask() {
    m_tasks                  = {};
    m_dag                    = nullptr;
    m_task_completion_states = {};
}

void hthread::IThreadWorkflowTask::set_workflow_metadata(
    ThreadWorkflowTasksView          tasks,
    ThreadWorkflowTaskID             task_idx,
    ThreadWorkflowDAG*               dag,
    ThreadWorkflowTaskCompletionView task_completion_states
) {
    m_tasks                  = tasks;
    m_task_idx               = task_idx;
    m_dag                    = dag;
    m_task_completion_states = task_completion_states;
}

bool hthread::IThreadWorkflowTask::execute() {
    if (run_task() && m_dag) {
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
                // TODO(Matthew): need to work out how to enable such chaining of tasks
                //                but we may also just ditch this as we probably
                //                just want to do this via events.
                (*queue_task)({ m_tasks.tasks.get()[next_task_idx].task,
                                m_tasks.tasks.get()[next_task_idx].delete_on_complete }
                );
            }
        }
    }

    // TODO(Matthew): allow workflow tasks to return need for continuation, failure, or
    //                success.

    return false;
}

void hthread::ThreadWorkflow::init(ThreadWorkflowDAG* dag, ThreadPool<>* thread_pool) {
    assert(dag != nullptr);
    assert(thread_pool != nullptr);

    m_dag         = dag;
    m_thread_pool = thread_pool;
}

void hthread::ThreadWorkflow::dispose() {
    m_dag         = nullptr;
    m_thread_pool = nullptr;
}

void hthread::ThreadWorkflow::run(ThreadWorkflowTasksView tasks) {
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
                                  tasks.tasks.get()[entry_task].delete_on_complete });
    }
}

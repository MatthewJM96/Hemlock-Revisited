template <hemlock::InterruptibleState ThreadState>
void hemlock::IThreadWorkflowTask<ThreadState>::execute(typename Thread<ThreadState>::State* state, TaskQueue<ThreadState>* task_queue) {
    
}

template <hemlock::InterruptibleState ThreadState>
hemlock::ThreadWorkflowTaskID hemlock::ThreadWorkflow<ThreadState>::add_task(HeldWorkflowTask<ThreadState>* task) {
    hemlock::ThreadWorkflowTaskID new_id = m_tasks.size();

    m_tasks.push_back(task);

    return new_id;
}

template <hemlock::InterruptibleState ThreadState>
hemlock::ThreadWorkflowTaskID hemlock::ThreadWorkflow<ThreadState>::add_tasks(HeldWorkflowTask<ThreadState>** tasks, ui32 count) {
    hemlock::ThreadWorkflowTaskID first_new_id = m_tasks.size();

    for (ui32 i = 0; i < count; ++i) {
        m_tasks.push_back(tasks[i]);
    }

    return new_id;
}

template <hemlock::InterruptibleState ThreadState>
hemlock::ThreadWorkflowTaskID hemlock::ThreadWorkflow<ThreadState>::chain_task(HeldWorkflowTask<ThreadState>* task) {
    hemlock::ThreadWorkflowTaskID new_id  = m_tasks.size();
    hemlock::ThreadWorkflowTaskID prev_id = new_id - 1;

    m_tasks.push_back(task);
    m_graph.push_back({prev_id, new_id});

    return new_id;
}

template <hemlock::InterruptibleState ThreadState>
hemlock::ThreadWorkflowTaskID hemlock::ThreadWorkflow<ThreadState>::chain_tasks(HeldWorkflowTask<ThreadState>* tasks, ui32 count) {
    hemlock::ThreadWorkflowTaskID first_new_id = m_tasks.size();

    hemlock::ThreadWorkflowTaskID new_id  = first_new_id;
    hemlock::ThreadWorkflowTaskID prev_id = new_id - 1;

    for (ui32 i = 0; i < count; ++i) {
        m_tasks.push_back(tasks[i]);
        m_graph.push_back({prev_id, new_id});

        ++prev_id; ++new_id;
    }

    return first_new_id;
}

template <hemlock::InterruptibleState ThreadState>
hemlock::ThreadWorkflowTaskID hemlock::ThreadWorkflow<ThreadState>::chain_tasks_parallel(HeldWorkflowTask<ThreadState>* tasks, ui32 count) {
    hemlock::ThreadWorkflowTaskID first_new_id = m_tasks.size();

    hemlock::ThreadWorkflowTaskID new_id  = first_new_id;
    hemlock::ThreadWorkflowTaskID prev_id = first_new_id - 1;

    for (ui32 i = 0; i < count; ++i) {
        m_tasks.push_back(tasks[i]);
        m_graph.push_back({prev_id, new_id});

        ++new_id;
    }

    return first_new_id;
}

template <hemlock::InterruptibleState ThreadState>
bool hemlock::ThreadWorkflow<ThreadState>::chain_task(ui32 first_task, ui32 second_task) {
    hemlock::ThreadWorkflowTaskID last_valid_id = m_tasks.size() - 1;
    if (first_task > last_valid_id || second_task > last_valid_id)
        return false;

    m_graph.push_back({first_task, second_task});

    return true;
}

template <hemlock::InterruptibleState ThreadState>
bool hemlock::ThreadWorkflow<ThreadState>::chain_tasks(std::pair<ui32, ui32>* task_pairs, ui32 count) {
    hemlock::ThreadWorkflowTaskID last_valid_id = m_tasks.size() - 1;

    // TODO(Matthew): Do we really like this?

    bool all_valid = true;

    for (ui32 i = 0; i < count; ++i) {
        if (first_task > last_valid_id || second_task > last_valid_id) {
            all_valid = false;
            continue;
        }

        m_graph.push_back({first_task, second_task});
    }

    return all_valid;
}

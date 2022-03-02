template <hemlock::InterruptibleState ThreadState>
void hemlock::IThreadWorkflowTask<ThreadState>::execute(typename Thread<ThreadState>::State* state, TaskQueue<ThreadState>* task_queue) {
    
}

template <hemlock::InterruptibleState ThreadState>
void hemlock::ThreadWorkflow<ThreadState>::init(ThreadPool<ThreadState>* thread_pool) {
    m_thread_pool = thread_pool;
}

template <hemlock::InterruptibleState ThreadState>
void hemlock::ThreadWorkflow<ThreadState>::dispose() {
    m_thread_pool = nullptr;

    for (auto task : m_tasks) {
        if (task.should_delete)
            delete task.task;
    }

    ThreadWorkflowTaskList<ThreadState>().swap(m_tasks);
    ThreadWorkflowTaskIndexList().swap(m_entry_tasks);
    ThreadWorkflowTaskGraph().swap(m_graph);
}

template <hemlock::InterruptibleState ThreadState>
void hemlock::ThreadWorkflow<ThreadState>::start() {
    
}

template <hemlock::InterruptibleState ThreadState>
void hemlock::ThreadWorkflow<ThreadState>::set_expected_tasks(ui32 expected_task_count) {
    m_tasks.reserve(expected_task_count);
    m_entry_tasks.reserve(expected_task_count);
}

template <hemlock::InterruptibleState ThreadState>
hemlock::ThreadWorkflowTaskID hemlock::ThreadWorkflow<ThreadState>::add_task(HeldWorkflowTask<ThreadState> task) {
    hemlock::ThreadWorkflowTaskID new_id = m_tasks.size();

    m_tasks.push_back(task);
    m_entry_tasks.insert(new_id);

    return new_id;
}

template <hemlock::InterruptibleState ThreadState>
hemlock::ThreadWorkflowTaskID hemlock::ThreadWorkflow<ThreadState>::add_tasks(HeldWorkflowTask<ThreadState>* tasks, ui32 count) {
    hemlock::ThreadWorkflowTaskID new_id = m_tasks.size();

    for (ui32 i = 0; i < count; ++i) {
        m_tasks.push_back(tasks[i]);
        m_entry_tasks.insert(new_id);

        ++new_id;
    }

    return new_id;
}

template <hemlock::InterruptibleState ThreadState>
hemlock::ThreadWorkflowTaskID hemlock::ThreadWorkflow<ThreadState>::chain_task(HeldWorkflowTask<ThreadState> task) {
    hemlock::ThreadWorkflowTaskID new_id  = m_tasks.size();
    hemlock::ThreadWorkflowTaskID prev_id = new_id - 1;

    if (new_id > 0) {
        m_graph.insert({prev_id, new_id});
    } else {
        m_entry_tasks.insert(new_id);
    }

    m_tasks.push_back(task);

    return new_id;
}

template <hemlock::InterruptibleState ThreadState>
hemlock::ThreadWorkflowTaskID hemlock::ThreadWorkflow<ThreadState>::chain_tasks(HeldWorkflowTask<ThreadState>* tasks, ui32 count) {
    hemlock::ThreadWorkflowTaskID first_new_id = m_tasks.size();

    hemlock::ThreadWorkflowTaskID new_id  = first_new_id;
    hemlock::ThreadWorkflowTaskID prev_id = new_id - 1;

    for (ui32 i = 0; i < count; ++i) {
        if (new_id > 0) {
            m_graph.insert({prev_id, new_id});
        } else {
            // Can only reach here if i == 0 and we had
            // not yet added any tasks to the workflow.
            m_entry_tasks.insert(new_id);
        }

        m_tasks.push_back(tasks[i]);

        prev_id = new_id++;
    }

    return first_new_id;
}

template <hemlock::InterruptibleState ThreadState>
hemlock::ThreadWorkflowTaskID hemlock::ThreadWorkflow<ThreadState>::chain_tasks_parallel(HeldWorkflowTask<ThreadState>* tasks, ui32 count) {
    hemlock::ThreadWorkflowTaskID first_new_id = m_tasks.size();

    hemlock::ThreadWorkflowTaskID new_id  = first_new_id;
    hemlock::ThreadWorkflowTaskID prev_id = first_new_id - 1;

    for (ui32 i = 0; i < count; ++i) {
        if (first_new_id > 0) {
            m_graph.insert({prev_id, new_id});
        } else {
            // Can only reach here if i == 0 and we had
            // not yet added any tasks to the workflow.
            m_entry_tasks.insert(new_id);
        }

        m_tasks.push_back(tasks[i]);

        ++new_id;
    }

    return first_new_id;
}

template <hemlock::InterruptibleState ThreadState>
bool hemlock::ThreadWorkflow<ThreadState>::chain_task(ThreadWorkflowTaskID first_task, ThreadWorkflowTaskID second_task) {
    hemlock::ThreadWorkflowTaskID last_valid_id = m_tasks.size() - 1;
    if (first_task > last_valid_id || second_task > last_valid_id)
        return false;

    // Second task is no longer an entry task if it was until now.
    if (auto it = m_entry_tasks.find(second_task); it != m_entry_tasks.end())
        m_entry_tasks.erase(it);

    m_graph.insert({first_task, second_task});

    return true;
}

template <hemlock::InterruptibleState ThreadState>
bool hemlock::ThreadWorkflow<ThreadState>::chain_tasks(std::pair<ThreadWorkflowTaskID, ThreadWorkflowTaskID>* task_pairs, ui32 count) {
    hemlock::ThreadWorkflowTaskID last_valid_id = m_tasks.size() - 1;

    // TODO(Matthew): Do we really like this?

    bool all_valid = true;

    for (ui32 i = 0; i < count; ++i) {
        auto [first_task, second_task] = task_pairs[i];

        if (first_task > last_valid_id || second_task > last_valid_id) {
            all_valid = false;
            continue;
        }

        // Second task is no longer an entry task if it was until now.
        if (auto it = m_entry_tasks.find(second_task); it != m_entry_tasks.end())
            m_entry_tasks.erase(it);

        m_graph.insert({first_task, second_task});
    }

    return all_valid;
}

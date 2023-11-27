#include "stdafx.h"

#include "thread/workflow/builder.h"

hthread::ThreadWorkflowBuilder::ThreadWorkflowBuilder() : m_dag(nullptr) { /* Empty. */
}

void hthread::ThreadWorkflowBuilder::init(ThreadWorkflowDAG* dag) {
    m_dag = dag;
}

void hthread::ThreadWorkflowBuilder::dispose() {
    m_dag = nullptr;
}

void hthread::ThreadWorkflowBuilder::set_expected_tasks(ui32 expected_task_count) {
    m_dag->into_counts.reserve(expected_task_count);
}

hthread::ThreadWorkflowTaskID hthread::ThreadWorkflowBuilder::add_task() {
    ThreadWorkflowTaskID new_id = m_dag->task_count;

    m_dag->task_count += 1;
    m_dag->into_counts.emplace_back(0);

    return new_id;
}

hthread::ThreadWorkflowTaskID hthread::ThreadWorkflowBuilder::add_tasks(ui32 count) {
    ThreadWorkflowTaskID first_new_id = m_dag->task_count;

    m_dag->task_count += count;
    m_dag->into_counts.reserve(count);
    for (ui32 i = 0; i < count; ++i) m_dag->into_counts.emplace_back(0);

    return first_new_id;
}

hthread::ThreadWorkflowTaskID hthread::ThreadWorkflowBuilder::chain_task() {
    // TODO(Matthew): this logic looks likely to underflow...

    ThreadWorkflowTaskID new_id  = m_dag->task_count;
    ThreadWorkflowTaskID prev_id = new_id - 1;

    m_dag->task_count += 1;
    if (new_id > 0) {
        m_dag->into_counts.emplace_back(1);
        m_dag->graph.insert({ prev_id, new_id });
    } else {
        m_dag->into_counts.emplace_back(0);
        m_dag->entry_tasks.insert(new_id);
    }

    return new_id;
}

hthread::ThreadWorkflowTaskID hthread::ThreadWorkflowBuilder::chain_tasks(ui32 count) {
    ThreadWorkflowTaskID first_new_id = m_dag->task_count;

    ThreadWorkflowTaskID new_id  = first_new_id;
    ThreadWorkflowTaskID prev_id = new_id - 1;

    m_dag->task_count += count;
    m_dag->into_counts.reserve(count);
    for (ui32 i = 0; i < count; ++i) {
        if (new_id > 0) {
            m_dag->into_counts.emplace_back(1);
            m_dag->graph.insert({ prev_id, new_id });
        } else {
            // Can only reach here if i == 0 and we had
            // not yet added any tasks to the workflow.
            m_dag->into_counts.emplace_back(0);
            m_dag->entry_tasks.insert(new_id);
        }

        prev_id = new_id++;
    }

    return first_new_id;
}

hthread::ThreadWorkflowTaskID
hthread::ThreadWorkflowBuilder::chain_tasks_parallel(ui32 count) {
    ThreadWorkflowTaskID first_new_id = m_dag->task_count;

    ThreadWorkflowTaskID new_id  = first_new_id;
    ThreadWorkflowTaskID prev_id = new_id - 1;

    m_dag->task_count += count;
    m_dag->into_counts.reserve(count);
    for (ui32 i = 0; i < count; ++i) {
        if (first_new_id > 0) {
            m_dag->into_counts.emplace_back(1);
            m_dag->graph.insert({ prev_id, new_id });
        } else {
            // Can only reach here if we had not yet
            // added any tasks to the workflow.
            m_dag->into_counts.emplace_back(0);
            m_dag->entry_tasks.insert(new_id);
        }

        ++new_id;
    }

    return first_new_id;
}

std::tuple<bool, hthread::ThreadWorkflowTaskID>
hthread::ThreadWorkflowBuilder::chain_task(ThreadWorkflowTaskID from_task) {
    ThreadWorkflowTaskID new_id = m_dag->task_count;
    if (from_task >= new_id) return { false, 0 };

    m_dag->task_count += 1;
    m_dag->into_counts.emplace_back(1);
    m_dag->graph.insert({ from_task, new_id });

    return { true, new_id };
}

std::tuple<bool, hthread::ThreadWorkflowTaskID>
hthread::ThreadWorkflowBuilder::chain_tasks(
    ThreadWorkflowTaskID from_task, ui32 count
) {
    ThreadWorkflowTaskID first_new_id = m_dag->task_count;
    if (from_task >= first_new_id) return { false, 0 };

    m_dag->task_count += count;
    m_dag->into_counts.reserve(count);

    ThreadWorkflowTaskID new_id = first_new_id;

    m_dag->into_counts.emplace_back(1);
    m_dag->graph.insert({ from_task, new_id });

    ThreadWorkflowTaskID prev_id = new_id++;

    for (ui32 i = 1; i < count; ++i) {
        m_dag->into_counts.emplace_back(1);
        m_dag->graph.insert({ prev_id, new_id });

        prev_id = new_id++;
    }

    return { true, first_new_id };
}

std::tuple<bool, hthread::ThreadWorkflowTaskID>
hthread::ThreadWorkflowBuilder::chain_tasks_parallel(
    ThreadWorkflowTaskID from_task, ui32 count
) {
    ThreadWorkflowTaskID first_new_id = m_dag->task_count;
    if (from_task >= first_new_id) return { false, 0 };

    m_dag->task_count += count;
    m_dag->into_counts.reserve(count);

    ThreadWorkflowTaskID new_id = first_new_id;

    for (ui32 i = 0; i < count; ++i) {
        m_dag->into_counts.emplace_back(1);
        m_dag->graph.insert({ from_task, new_id });

        ++new_id;
    }

    return { true, first_new_id };
}

bool hthread::ThreadWorkflowBuilder::set_task_depends(
    ThreadWorkflowTaskID first_task, ThreadWorkflowTaskID second_task
) {
    hthread::ThreadWorkflowTaskID next_valid_id = m_dag->task_count;
    if (first_task >= next_valid_id || second_task >= next_valid_id) return false;

    // Second task is no longer an entry task if it was until now.
    if (auto it = m_dag->entry_tasks.find(second_task); it != m_dag->entry_tasks.end())
        m_dag->entry_tasks.erase(it);

    m_dag->into_counts[second_task] += 1;
    m_dag->graph.insert({ first_task, second_task });

    return true;
}

bool hthread::ThreadWorkflowBuilder::set_tasks_depend(
    std::pair<ThreadWorkflowTaskID, ThreadWorkflowTaskID>* task_pairs, ui32 count
) {
    hthread::ThreadWorkflowTaskID next_valid_id = m_dag->task_count;

    // TODO(Matthew): Do we really like this?

    bool all_valid = true;

    for (ui32 i = 0; i < count; ++i) {
        auto [first_task, second_task] = task_pairs[i];

        if (first_task >= next_valid_id || second_task >= next_valid_id) {
            all_valid = false;
            continue;
        }

        // Second task is no longer an entry task if it was until now.
        if (auto it = m_dag->entry_tasks.find(second_task);
            it != m_dag->entry_tasks.end())
            m_dag->entry_tasks.erase(it);

        m_dag->into_counts[second_task] += 1;
        m_dag->graph.insert({ first_task, second_task });
    }

    return all_valid;
}

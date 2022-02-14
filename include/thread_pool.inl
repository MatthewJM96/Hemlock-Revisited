template <hemlock::InterruptibleState ThreadState>
void hemlock::basic_thread_main( typename Thread<ThreadState>::State* state,
                                              TaskQueue<ThreadState>* task_queue  ) {
    state->context.stop    = false;
    state->context.suspend = false;

    HeldTask<ThreadState> held = {nullptr, false};
    while (!state->context.stop) {
        task_queue->wait_dequeue_timed(
            state->consumer_token,
            held,
            std::chrono::seconds(1)
        );

        // TODO(Matthew): This is bad, but it'll work for now.
        while (state->context.suspend)
            continue;

        if (!held.task) continue;

        held.task->execute(state, task_queue);
        held.task->is_finished = true;
        held.task->dispose();
        if (held.should_delete) delete held.task;
        held.task = nullptr;
    }
}

template <hemlock::InterruptibleState ThreadState>
void hemlock::ThreadPool<ThreadState>::init(           ui32 thread_count,
                                ThreadMainFunc<ThreadState> thread_main_func /*= {basic_thread_main}*/  )
{
    if (m_is_initialised) return;
    m_is_initialised = true;

    m_thread_main_func = thread_main_func;

    m_producer_token = moodycamel::ProducerToken(m_tasks);

    m_threads.reserve(thread_count);
    for (ui32 i = 0; i < thread_count; ++i) {
        m_threads.emplace_back(Thread<ThreadState>{
            .thread = std::thread(
                m_thread_main_func,
                reinterpret_cast<Thread<ThreadState>::State*>(
                    reinterpret_cast<ui8*>(&m_threads[i]) + offsetof(Thread<ThreadState>, state)
                ),
                &m_tasks
            ),
            .state {
                .consumer_token = moodycamel::ConsumerToken(m_tasks),
                .producer_token = moodycamel::ProducerToken(m_tasks)
            }
        });
    }
}

template <hemlock::InterruptibleState ThreadState>
void hemlock::ThreadPool<ThreadState>::dispose() {
    if (!m_is_initialised) return;
    m_is_initialised = false;

    for (auto& thread : m_threads) {
        thread.state.context.stop    = true;
        // Necessary to let threads come to an end of
        // execution.
        thread.state.context.suspend = false;
    }

    for (auto& thread : m_threads)
        thread.thread.join();

    TaskQueue<ThreadState>().swap(m_tasks);

    Threads<ThreadState>().swap(m_threads);
}

template <hemlock::InterruptibleState ThreadState>
void hemlock::ThreadPool<ThreadState>::suspend() {
    for (auto& thread : m_threads)
        thread.state.context.suspend = true;
}

template <hemlock::InterruptibleState ThreadState>
void hemlock::ThreadPool<ThreadState>::resume() {
    for (auto& thread : m_threads)
        thread.state.context.suspend = false;
}

template <hemlock::InterruptibleState ThreadState>
void hemlock::ThreadPool<ThreadState>::add_task(HeldTask<ThreadState> task) {
    m_tasks.enqueue(m_producer_token, task);
}

template <hemlock::InterruptibleState ThreadState>
void hemlock::ThreadPool<ThreadState>::add_tasks(HeldTask<ThreadState> tasks[], size_t task_count) {
    m_tasks.enqueue_bulk(m_producer_token, tasks, task_count);
}

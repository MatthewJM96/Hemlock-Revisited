template <
    hthread::IsThreadState              ThreadState,
    hthread::ThreadpoolTimingResolution Timing>
void hthread::ThreadPool<ThreadState, Timing>::init(
    ui32                                thread_count,
    ThreadMainFunc<ThreadState, Timing> thread_main_func /*= {basic_thread_main}*/
) {
    if (m_is_initialised) return;
    m_is_initialised = true;

    m_thread_main_func = thread_main_func;

    m_producer_token = moodycamel::ProducerToken(m_tasks);

    m_threads.reserve(thread_count);
    for (ui32 i = 0; i < thread_count; ++i) {
        m_threads.emplace_back(Thread<ThreadState>{
            .thread = std::thread(
                m_thread_main_func,
                reinterpret_cast<ThreadState*>(
                    reinterpret_cast<ui8*>(&m_threads.data()[i])
                    + offsetof(Thread<ThreadState>, state)
                ),
                &m_tasks
            ),
            .state{.consumer_token = moodycamel::ConsumerToken(m_tasks),
                   .producer_token = moodycamel::ProducerToken(m_tasks),
                   .stop           = false,
                   .suspend        = false}
        });
    }
}

template <
    hthread::IsThreadState              ThreadState,
    hthread::ThreadpoolTimingResolution Timing>
void hthread::ThreadPool<ThreadState, Timing>::dispose() {
    if (!m_is_initialised) return;
    m_is_initialised = false;

    for (auto& thread : m_threads) {
        thread.state.stop = true;
        // Necessary to let threads come to an end of
        // execution.
        thread.state.suspend = false;
    }

    for (auto& thread : m_threads) thread.thread.join();

    TaskQueue<ThreadState>().swap(m_tasks);

    Threads<ThreadState>().swap(m_threads);
}

template <
    hthread::IsThreadState              ThreadState,
    hthread::ThreadpoolTimingResolution Timing>
void hthread::ThreadPool<ThreadState, Timing>::suspend() {
    for (auto& thread : m_threads) thread.state.suspend = true;
}

template <
    hthread::IsThreadState              ThreadState,
    hthread::ThreadpoolTimingResolution Timing>
void hthread::ThreadPool<ThreadState, Timing>::resume() {
    for (auto& thread : m_threads) thread.state.suspend = false;
}

template <
    hthread::IsThreadState              ThreadState,
    hthread::ThreadpoolTimingResolution Timing>
void hthread::ThreadPool<ThreadState, Timing>::add_task(HeldTask<ThreadState> task) {
    m_tasks.enqueue(m_producer_token, task);
}

template <
    hthread::IsThreadState              ThreadState,
    hthread::ThreadpoolTimingResolution Timing>
void hthread::ThreadPool<ThreadState, Timing>::add_tasks(
    HeldTask<ThreadState> tasks[], size_t task_count
) {
    m_tasks.enqueue_bulk(m_producer_token, tasks, task_count);
}

template <
    hthread::IsThreadState              ThreadState,
    hthread::ThreadpoolTimingResolution Timing>
void hthread::ThreadPool<ThreadState, Timing>::threadsafe_add_task(
    HeldTask<ThreadState> task
) {
    m_tasks.enqueue(task);
}

template <
    hthread::IsThreadState              ThreadState,
    hthread::ThreadpoolTimingResolution Timing>
void hthread::ThreadPool<ThreadState, Timing>::threadsafe_add_tasks(
    HeldTask<ThreadState> tasks[], size_t task_count
) {
    m_tasks.enqueue_bulk(tasks, task_count);
}

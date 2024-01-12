template <hthread::IsTaskQueue TaskQueue>
void hthread::ThreadPool<TaskQueue>::init(
    ui32                      thread_count,
    ThreadMainFunc<TaskQueue> thread_main_func /*= {default_thread_main}*/
) {
    if (m_is_initialised) return;
    m_is_initialised = true;

    m_thread_main_func = thread_main_func;

    m_threads.reserve(thread_count);
    for (ui32 i = 0; i < thread_count; ++i) {
        m_threads.emplace_back(Thread{
            .thread = std::thread(
                m_thread_main_func,
                reinterpret_cast<ThreadState*>(
                    reinterpret_cast<ui8*>(&m_threads.data()[i])
                    + offsetof(Thread, state)
                ),
                &m_tasks
            ),
            .state{.stop = false, .suspend = false}
        });
    }
}

template <hthread::IsTaskQueue TaskQueue>
void hthread::ThreadPool<TaskQueue>::dispose() {
    if (!m_is_initialised) return;
    m_is_initialised = false;

    for (auto& thread : m_threads) {
        thread.state.stop = true;
        // Necessary to let threads come to an end of
        // execution.
        thread.state.suspend = false;
    }

    for (auto& thread : m_threads) thread.thread.join();

    TaskQueue().swap(m_tasks);

    Threads().swap(m_threads);
}

template <hthread::IsTaskQueue TaskQueue>
void hthread::ThreadPool<TaskQueue>::suspend() {
    for (auto& thread : m_threads) thread.state.suspend = true;
}

template <hthread::IsTaskQueue TaskQueue>
void hthread::ThreadPool<TaskQueue>::resume() {
    for (auto& thread : m_threads) thread.state.suspend = false;
}

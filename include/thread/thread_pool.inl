template <
    hthread::IsTaskQueue                TaskQueue,
    bool Timing>
void hthread::ThreadPool<TaskQueue, Timing>::init(
    ui32 thread_count,
    ThreadMainFunc<TaskQueue, Timing>
        thread_main_func /*= {basic_thread_main}*/
) {
    if (m_is_initialised) return;
    m_is_initialised = true;

    m_thread_main_func = thread_main_func;

    m_producer_token = moodycamel::ProducerToken(m_tasks);

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

template <
    hthread::IsTaskQueue                TaskQueue,
    bool Timing>
void hthread::ThreadPool<TaskQueue, Timing>::dispose() {
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

template <
    hthread::IsTaskQueue                TaskQueue,
    bool Timing>
void hthread::ThreadPool<TaskQueue, Timing>::suspend() {
    for (auto& thread : m_threads) thread.state.suspend = true;
}

template <
    hthread::IsTaskQueue                TaskQueue,
    bool Timing>
void hthread::ThreadPool<TaskQueue, Timing>::resume() {
    for (auto& thread : m_threads) thread.state.suspend = false;
}

template <
    hthread::IsTaskQueue                TaskQueue,
    bool Timing>
void hthread::ThreadPool<TaskQueue, Timing>::add_task(QueuedTask task) {
    m_tasks.enqueue(m_producer_token, task);
}

template <
    hthread::IsTaskQueue                TaskQueue,
    bool Timing>
void hthread::ThreadPool<TaskQueue, Timing>::add_tasks(
    QueuedTask tasks[], size_t task_count
) {
    m_tasks.enqueue_bulk(m_producer_token, tasks, task_count);
}

template <
    hthread::IsTaskQueue                TaskQueue,
    bool Timing>
void hthread::ThreadPool<TaskQueue, Timing>::threadsafe_add_task(
    QueuedTask task
) {
    m_tasks.enqueue(task);
}

template <
    hthread::IsTaskQueue                TaskQueue,
    bool Timing>
void hthread::ThreadPool<TaskQueue, Timing>::threadsafe_add_tasks(
    QueuedTask tasks[], size_t task_count
) {
    m_tasks.enqueue_bulk(tasks, task_count);
}

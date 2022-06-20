template <hthread::InterruptibleState ThreadState>
void hthread::basic_thread_main( typename Thread<ThreadState>::State* state,
                                              TaskQueue<ThreadState>* task_queue  ) {
    state->context.stop    = false;
    state->context.suspend = false;

    // NOTE(Matthew): Technically can get an exception here if we attempt
    //                a dequeue before the queue is fully constructed.
    //                I think as the semaphore wait causes an early return
    //                on timeout, we avoid it with the blocking approach.
    //                  This issue will definitely come up if we were to
    //                  use the non-blocking queue. That said, we shouldn't
    //                  do that unless we're absolutely sure the partial
    //                  spin pattern used in lightweight semaphore is
    //                  doing anything bad to us. This is doubtful.

    HeldTask<ThreadState> held = {nullptr, false};
    while (!state->context.stop) {
        task_queue->wait_dequeue_timed(
            state->consumer_token,
            held,
            std::chrono::seconds(1)
        );

        while (state->context.suspend)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (!held.task) {
            std::this_thread::yield();
            continue;
        }

        held.task->execute(state, task_queue);
        held.task->is_finished = true;
        held.task->dispose();
        if (held.should_delete) delete held.task;
        held.task = nullptr;
    }
}

template <hthread::InterruptibleState ThreadState>
void hthread::ThreadPool<ThreadState>::init(           ui32 thread_count,
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
                reinterpret_cast<typename Thread<ThreadState>::State*>(
                    reinterpret_cast<ui8*>(&m_threads.data()[i]) + offsetof(Thread<ThreadState>, state)
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

template <hthread::InterruptibleState ThreadState>
void hthread::ThreadPool<ThreadState>::dispose() {
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

template <hthread::InterruptibleState ThreadState>
void hthread::ThreadPool<ThreadState>::suspend() {
    for (auto& thread : m_threads)
        thread.state.context.suspend = true;
}

template <hthread::InterruptibleState ThreadState>
void hthread::ThreadPool<ThreadState>::resume() {
    for (auto& thread : m_threads)
        thread.state.context.suspend = false;
}

template <hthread::InterruptibleState ThreadState>
void hthread::ThreadPool<ThreadState>::add_task(HeldTask<ThreadState> task) {
    m_tasks.enqueue(m_producer_token, task);
}

template <hthread::InterruptibleState ThreadState>
void hthread::ThreadPool<ThreadState>::add_tasks(HeldTask<ThreadState> tasks[], size_t task_count) {
    m_tasks.enqueue_bulk(m_producer_token, tasks, task_count);
}

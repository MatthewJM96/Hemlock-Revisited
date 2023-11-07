#ifndef __hemlock_thread_basic_thread_main_hpp
#define __hemlock_thread_basic_thread_main_hpp

namespace hemlock {
    namespace thread {
        /**
         * @brief A basic main function of threads.
         *
         * @param state The thread state, including tokens for
         * interacting with task queue, and thread pool specific
         * context.
         * @param task_queue The task queue, can be interacted with
         * for example if a task needs to chain a follow-up task.
         */
        template <IsThreadState ThreadState, ThreadpoolTimingResolution Timing>
        void basic_thread_main(ThreadState* state, TaskQueue<ThreadState>* task_queue) {
            state->stop    = false;
            state->suspend = false;

            // NOTE(Matthew): Technically can get an exception here if we attempt
            //                a dequeue before the queue is fully constructed.
            //                I think as the semaphore wait causes an early return
            //                on timeout, we avoid it with the blocking approach.
            //                  This issue will definitely come up if we were to
            //                  use the non-blocking queue. That said, we shouldn't
            //                  do that unless we're absolutely sure the partial
            //                  spin pattern used in lightweight semaphore is
            //                  doing anything bad to us. This is doubtful.

            HeldTask<ThreadState> held = { nullptr, false };
            while (!state->stop) {
                task_queue->wait_dequeue_timed(
                    state->consumer_token, held, std::chrono::seconds(1)
                );

                while (state->suspend)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                if (!held.task) {
                    std::this_thread::yield();
                    continue;
                }

                if (held.task->execute(state, task_queue)) {
                    held.task->dispose();
                    if (held.should_delete) delete held.task;
                    held.task = nullptr;
                } else {
                    task_queue->enqueue(state->producer_token, std::move(held));
                }
            }
        }
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_basic_thread_main_hpp

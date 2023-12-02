#ifndef __hemlock_thread_basic_thread_main_hpp
#define __hemlock_thread_basic_thread_main_hpp

#include "thread/state.hpp"
#include "thread/queue/state.hpp"

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
        template <IsTimedTaskQueue TaskQueue>
        void timed_thread_main(ThreadState* state, TaskQueue* task_queue) {
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

            QueuedTask current_task = { nullptr, false };
            while (!state->stop) {
                auto [queue_task, register_timing]
                    = task_queue->dequeue(current_task, std::chrono::microseconds(100));

                while (state->suspend)
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));

                if (!current_task.task) {
                    std::this_thread::yield();
                    continue;
                }

                auto task = reinterpret_cast<ThreadTaskBase*>(current_task.task);

                auto before = std::chrono::system_clock::now();
                bool task_completion = task->execute(&queue_task);
                auto duration = std::chrono::system_clock::now() - before;

                register_timing(task_completion, duration.count());

                if (task_completion) {
                    // Task completed, handle disposal.
                    task->dispose();
                    if (current_task.delete_on_complete) delete task;
                    current_task.task = nullptr;
                } else {
                    // Task did not complete, requeue it.

                    queue_task(std::move(current_task));
                }
            }
        }
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_basic_thread_main_hpp

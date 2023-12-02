#ifndef __hemlock_thread_queue_multi_queue_hpp
#define __hemlock_thread_queue_multi_queue_hpp

#include "thread/state.hpp"

namespace hemlock {
    namespace thread {
        template <size_t QueueCount>
        using FixedCountQueues = std::array<moodycamel::BlockingConcurrentQueue<QueuedTask>, QueueCount>;

        template <size_t QueueCount>
        using ChangeQueueQuery = Delegate<bool(FixedCountQueues<QueueCount>, size_t)>;

        // NOTES
        //
        // Perhaps we shouldn't use a query... we want this to just go in transparently
        // into a threadpool even if it uses the basic main function. To achieve this we
        // do need to change the queues outside of the threadpool logic, but the dequeue
        // function still seems wrong as bookkeeping is then enforced on the side of the
        // callee of the query to ensure multiple threads don't try to change
        // m_curr_index. That said, to have a function that changes it externally still
        // seems wrong as then we don't satisfy the checkerboard update pattern which
        // is one thing we want to support here.
        //   We should first sketch out two patterns, the checkerboard pattern and the
        //   timing-based case. Perhaps we find these need to be separate
        //   implementations entirely.
        //
        // Checkboard Update Pattern
        //
        //   Update curr_index after:
        //    - exhausting current checker's tasks
        //        this needs some conception of a given thread's timeout being reached
        //        and this causes curr_index to update, but then we need to verify in
        //        all threads after dequeueing a task that the curr_index remains the
        //        same AT LEAST right now.
        //          how do we deal with the case of a long-running task in one thread
        //          but another thread saying "yeah let's look at another checker now"
        //          at that point we still have to deal with locking unless we do some
        //          kind of synchronisation here.
        //    - after processing some number of tasks or some amount of time elapsing in
        //      processing those tasks.
        //
        // Timing-based Pattern
        //
        //   Update curr_index after:
        //    - current queue has lost priority according to EEVDF.
        //        note that EEVDF is a per-task algorithm, we are using queues therefore
        //        to represent BOTH a prioritisation and a time-requirement claim.


        template <size_t QueueCount>
        class MultiTaskQueue {
        public:
            QueueDelegate dequeue(QueuedTask& task, std::chrono::microseconds timeout) {
                wait_dequeue_timed(task, timeout);

                return QueueDelegate{ [this](QueuedTask&& task) {
                    return this->enqueue(std::forward<QueuedTask>(task));
                } };
            }

        protected:
            ChangeQueueQuery<QueueCount> should_change_queue;

            volatile size_t              m_curr_index;
            FixedCountQueues<QueueCount> m_queues;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_multi_queue_hpp

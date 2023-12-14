#ifndef __hemlock_thread_queue_prioritised_multi_queue_hpp
#define __hemlock_thread_queue_prioritised_multi_queue_hpp

#include "thread/queue/basic.hpp"
#include "thread/queue/state.hpp"
#include "thread/state.hpp"

namespace hemlock {
    namespace thread {
        class PrioritisedMultiTaskQueue {
        public:
            using IdentifierType = size_t;

            bool dequeue(QueuedTask& task, TimingRep timeout, void* identifier) {
                *identifier = m_curr_index;

                return m_queues[m_curr_index].wait_dequeue_timed(task, timeout);
            }

            bool queue(QueuedTask task, void* identifier) { }

            void register_timing(QueuedTask& task, TimingRep timing, void* identifier) {

            }
        protected:
            volatile size_t             m_curr_index;
            std::vector<BasicTaskQueue> m_queues;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_prioritised_multi_queue_hpp

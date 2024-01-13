#ifndef __hemlock_thread_queue_priority_task_queue
#define __hemlock_thread_queue_priority_task_queue

#include "thread/queue/state.hpp"

namespace hemlock {
    namespace thread {
        template <IsTaskQueue... Queues>
        class PriorityTaskQueue {
        public:
            struct ControlBlock {
                ControlBlock(PriorityTaskQueue* queue) {
                    for (size_t idx = 0; idx < sizeof...(Queues); ++idx) {
                        std::get<idx>(control_blocks) = { std::get<idx>(m_queues) };
                    }
                };

                typename std::tuple<typename Queues::ControlBlock...> control_blocks;
            };

            void set_priority(size_t index, size_t priority) {
                m_priorities[index] = priority;
            }

            void register_task_queue();

            bool dequeue(QueuedTask& task, TimingRep timeout, void*& identifier) {
                *identifier = m_curr_index;

                return m_queues[m_curr_index].wait_dequeue_timed(task, timeout);
            }

            bool queue(QueuedTask task, void* identifier) { }

            void register_timing(QueuedTask& task, TimingRep timing, void* identifier) {

            }
        protected:
            std::tuple<Queues...>              m_queues;
            std::unordered_map<size_t, size_t> m_priorities;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_priority_task_queue

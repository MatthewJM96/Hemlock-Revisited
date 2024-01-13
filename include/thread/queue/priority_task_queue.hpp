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

            ControlBlock* register_thread() { return new ControlBlock{ this }; }

            ControlBlock* register_threads(size_t count) {
                // ProducerToken and ConsumerToken have no default constructor.

                void*         mem            = new ui8[count * sizeof(ControlBlock)];
                ControlBlock* control_blocks = reinterpret_cast<ControlBlock*>(mem);

                for (size_t idx = 0; idx < count; ++idx) {
                    control_blocks[idx] = ControlBlock{ this };
                }

                return control_blocks;
            }

            bool dequeue(
                QueuedTask*      item,
                TimingRep        timeout,
                BasicTaskQueue** queue,
                void*            control_block
            ) {
                // TODO(Matthew): Implement a priority algorithm here.... actually do it
                //                as a templated strategy!!!
                (void)item;
                (void)timeout;
                (void)queue;
                (void)control_block;

                return false;
            }
        protected:
            std::tuple<Queues...>              m_queues;
            std::unordered_map<size_t, size_t> m_priorities;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_priority_task_queue

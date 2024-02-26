#ifndef __hemlock_thread_queue_checker_task_queue_hpp
#define __hemlock_thread_queue_checker_task_queue_hpp

#include "thread/queue/state.h"

namespace hemlock {
    namespace thread {
        template <size_t CheckerCount, IsBasicTaskQueue QueueType = BasicTaskQueue>
        class CheckerTaskQueue {
        public:
            CheckerTaskQueue() : m_queue{} {
                // Empty.
            }

            struct ControlBlock {
                ControlBlock(CheckerTaskQueue* queue) {
                    for (size_t idx = 0; idx < CheckerCount; ++idx) {
                        control_blocks[idx] =
                            typename QueueType::ControlBlock{ &queue->m_queues[idx] };
                    }
                }

                std::array<QueueType::ControlBlock, CheckerCount> control_blocks;
            };

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

            bool enqueue(size_t idx, const QueuedTask& item, void* control_block) {
                auto& queue = m_queues[idx];
                auto& underlying_control_block
                    = reinterpret_cast<ControlBlock*>(control_block)
                          ->control_blocks[idx];

                return queue.enqueue(item, &underlying_control_block);
            }

            bool enqueue(size_t idx, QueuedTask&& item, void* control_block) {
                auto& queue = m_queues[idx];
                auto& underlying_control_block
                    = reinterpret_cast<ControlBlock*>(control_block)
                          ->control_blocks[idx];

                return queue.enqueue(
                    std::forward<QueuedTask>(item), underlying_control_block
                );
            }

            // TODO(Matthew): here and in priority task queue, do we return a limited
            //                view on the underlying queue a task came from that only
            //                permits an enqueue operation? We really really don't want
            //                any dequeues from underlyings of this queue especially,
            //                not so important in the case of the priority task queue.
            bool dequeue(
                OUT QueuedTask*      item,
                TimingRep            timeout,
                OUT BasicTaskQueue** queue,
                void*                control_block
            ) {
                // TODO(Matthew): implement

                // Determine if we should at this point shift to the next queue.

                // If so, we cannot dequeue until previous tasks have been fully
                // consumed.
                // TODO(Matthew): how do we track the consumption of a task? Do we
                // decorate tasks with something that calls back to this queue to let it
                // know?

                // Once we can, dequeue from the queue we should dequeue from.
                //   Handle appropriate timeout.
            }
        protected:
            std::array<QueueType, CheckerCount> m_queues;
        };

        template <size_t CheckerCount, IsBasicTaskQueue QueueType>
        bool dequeue(
            CheckerTaskQueue<CheckerCount, QueuesType>& queue,
            OUT QueuedTask*                             item,
            TimingRep                                   timeout,
            OUT BasicTaskQueue**                        queue_out,
            void*                                       control_block
        ) {
            return queue.dequeue(item, timeout, queue_out, control_block);
        }
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_checker_task_queue_hpp

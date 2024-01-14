#ifndef __hemlock_thread_queue_priority_task_queue
#define __hemlock_thread_queue_priority_task_queue

#include "thread/queue/state.hpp"

namespace hemlock {
    namespace thread {
        using Priorities = std::map<size_t, size_t>;

        /**
         * @brief Defines a struct or class whose opeartor() evaluates which underlying
         * queue to dequeue from. If that queue is empty, the next lower priority queue
         * is pulled from and so on.
         */
        template <typename Candidate>
        concept IsPriorityTaskQueueStrategy
            = requires { typename Candidate::State; }
              && requires (
                  Candidate                  candidate,
                  typename Candidate::State* state,
                  const Priorities&          priorities
              ) {
                     {
                         candidate.operator()(state, priorities)
                         } -> std::same_as<Priorities::const_iterator>;
                 };

        template <IsPriorityTaskQueueStrategy Strategy, IsTaskQueue... Queues>
        class GenericPriorityTaskQueue {
        public:
            GenericPriorityTaskQueue() : m_queues{}, m_priorities{}, m_state{} {
                // Empty.
            }

            struct ControlBlock {
                template <size_t... Indices>
                void
                init_control_blocks(GenericPriorityTaskQueue* queue, std::index_sequence<Indices...>) {
                    control_blocks = {({ std::get<Indices>(queue->m_queues) }, ...) }
                }

                template <size_t... Indices>
                void init_control_block_ptrs(std::index_sequence<Indices...>) {
                    control_block_ptrs
                        = {({ &std::get<Indices>(control_blocks) }, ...) }
                }

                ControlBlock(GenericPriorityTaskQueue* queue) {
                    init_control_blocks(
                        queue, std::make_index_sequence<sizeof...(Queues)>()
                    );

                    init_control_block_ptrs(std::make_index_sequence<sizeof...(Queues)>(
                    ));
                }

                std::array<void*, sizeof...(Queues)> control_block_ptrs;
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
                OUT QueuedTask*      item,
                TimingRep            timeout,
                OUT BasicTaskQueue** queue,
                void*                control_block
            ) {
                Priorities::const_iterator it = Strategy{}(&m_state, m_priorities);

                // TODO(Matthew): need to obtain the correct control block, here we do
                //                not achieve this, instead just passing the whole load
                //                of control blocks over to the underlying queue we are
                //                to try to dequeue from.

                QueuedTask tmp = {};
                invoke_at(
                    m_queues, it->second, dequeue, &tmp, timeout, queue, control_block
                );

                *item = tmp;

                if (tmp.task != nullptr) {
                    // TODO(Matthew): try next lowest priority queue, and so on til end,
                    //                then wrap round to highest. Once reaching the
                    //                queue we started at without obtaining a task,
                    //                return false.
                }
            }
        protected:
            std::tuple<Queues...>    m_queues;
            Priorities               m_priorities;
            typename Strategy::State m_state;
        };

        template <IsPriorityTaskQueueStrategy Strategy, IsTaskQueue... Queues>
        bool dequeue(
            GenericPriorityTaskQueue<Strategy, Queues...>& queue,
            OUT QueuedTask*                                item,
            TimingRep                                      timeout,
            OUT BasicTaskQueue**                           queue_out,
            void*                                          control_block
        ) {
            return queue.dequeue(item, timeout, queue_out, control_block);
        }

        template <IsTaskQueue... Queues>
        using PriorityTaskQueue = GenericPriorityTaskQueue<SomeStrategy, Queues...>;
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_priority_task_queue

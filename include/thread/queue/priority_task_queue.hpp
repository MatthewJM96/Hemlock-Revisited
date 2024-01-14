#ifndef __hemlock_thread_queue_priority_task_queue
#define __hemlock_thread_queue_priority_task_queue

#include "thread/queue/state.h"

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
                    control_blocks = { { std::get<Indices>(queue->m_queues) }... };
                }

                template <size_t... Indices>
                void init_control_block_ptrs(std::index_sequence<Indices...>) {
                    control_block_ptrs = { &std::get<Indices>(control_blocks)... };
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
                auto start_it = Strategy{}(&m_state, m_priorities);
                auto curr_it  = start_it;

                while (curr_it != m_priorities.end()) {
                    void* underyling_control_block
                        = reinterpret_cast<ControlBlock*>(control_block)
                              ->control_block_ptrs[curr_it->second];

                    QueuedTask tmp = {};
                    invoke_at(
                        m_queues,
                        curr_it->second,
                        dequeue,
                        &tmp,
                        timeout,
                        queue,
                        underyling_control_block
                    );

                    if (tmp.task != nullptr) {
                        *item = tmp;
                        return true;
                    }

                    ++curr_it;
                }

                curr_it = m_priorities.cbegin();

                while (curr_it != start_it) {
                    void* underyling_control_block
                        = reinterpret_cast<ControlBlock*>(control_block)
                              ->control_block_ptrs[curr_it->second];

                    QueuedTask tmp = {};
                    invoke_at(
                        m_queues,
                        curr_it->second,
                        dequeue,
                        &tmp,
                        timeout,
                        queue,
                        underyling_control_block
                    );

                    if (tmp.task != nullptr) {
                        *item = tmp;
                        return true;
                    }

                    ++curr_it;
                }

                return false;
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

        struct HighestPriorityStrategy {
            // TODO(Matthew): support stateless strategies.
            using State = ui8;

            Priorities::const_iterator
            operator()(State*, const Priorities& priorities) {
                return priorities.cbegin();
            }
        };

        template <IsTaskQueue... Queues>
        using PriorityTaskQueue
            = GenericPriorityTaskQueue<HighestPriorityStrategy, Queues...>;
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_priority_task_queue

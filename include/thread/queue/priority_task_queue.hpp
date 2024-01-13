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
                ControlBlock(GenericPriorityTaskQueue* queue) {
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
                Priorities::const_iterator it = Strategy{}(&m_state, m_priorities);

                // Note that to make this work we probably need to use actual
                // inheritance across queues so that we can say "use this in-common
                // function". At which point we probably don't even need to use tuple
                // any more as opposed to an array of pointers to the base type.
                invoke_on(m_queues, it->second, dequeue

                return false;
            }
        protected:
            std::tuple<Queues...>    m_queues;
            Priorities               m_priorities;
            typename Strategy::State m_state;
        };

        template <IsTaskQueue... Queues>
        using PriorityTaskQueue = GenericPriorityTaskQueue<SomeStrategy, Queues...>;
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_priority_task_queue

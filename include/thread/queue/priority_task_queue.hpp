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

        template <IsPriorityTaskQueueStrategy Strategy, IsBasicTaskQueue... Queues>
        class GenericPriorityTaskQueue {
        public:
            GenericPriorityTaskQueue() : m_queues{}, m_priorities{}, m_state{} {
                init_priorities(std::make_index_sequence<sizeof...(Queues)>());
            }

            struct ControlBlock {
                template <size_t... Indices>
                void
                init_control_blocks(GenericPriorityTaskQueue* queue, std::index_sequence<Indices...>) {
                    control_blocks =
                        typename std::tuple<typename Queues::ControlBlock...>(
                            typename Queues::ControlBlock{
                                &std::get<Indices>(queue->m_queues) }...
                        );
                }

                template <size_t... Indices>
                void init_control_block_ptrs(std::index_sequence<Indices...>) {
                    control_block_ptrs = { &std::get<Indices>(control_blocks)... };
                }

                ControlBlock(GenericPriorityTaskQueue* queue) : control_blocks{} {
                    init_control_blocks(
                        queue, std::make_index_sequence<sizeof...(Queues)>()
                    );

                    init_control_block_ptrs(std::make_index_sequence<sizeof...(Queues)>(
                    ));
                }

                typename std::tuple<typename Queues::ControlBlock...> control_blocks;
                std::array<void*, sizeof...(Queues)> control_block_ptrs;
            };

            void set_priority(size_t index, size_t priority) {
                // TODO(Matthew): we need to make this threadsafe... OR we require that
                //                the caller know not to change priorities once the
                //                queue is in use - this should probably be the case.

                auto it = std::find_if(
                    m_priorities.begin(),
                    m_priorities.end(),
                    [index](const auto& el) { return el->second == index; }
                );

                m_priorities.erase(it);
                m_priorities[priority] = index;
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

            template <size_t QueueIndex>
            bool enqueue(const QueuedTask& item, void* control_block) {
                auto& queue                    = std::get<QueueIndex>(m_queues);
                auto& underlying_control_block = std::get<QueueIndex>(
                    reinterpret_cast<ControlBlock*>(control_block)->control_blocks
                );

                return queue.enqueue(item, &underlying_control_block);
            }

            template <size_t QueueIndex>
            bool enqueue(QueuedTask&& item, void* control_block) {
                auto& queue                    = std::get<QueueIndex>(m_queues);
                auto& underlying_control_block = std::get<QueueIndex>(
                    reinterpret_cast<ControlBlock*>(control_block)->control_blocks
                );

                return queue.enqueue(
                    std::forward<QueuedTask>(item), underlying_control_block
                );
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
                        hthread::dequeue,
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
                        hthread::dequeue,
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
            template <size_t... Indices>
            void init_priorities(std::index_sequence<Indices...>) {
                m_priorities = Priorities{
                    {Indices, Indices}
                    ...
                };
            }

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

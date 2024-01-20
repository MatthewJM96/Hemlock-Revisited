#ifndef __hemlock_thread_queue_state_hpp
#define __hemlock_thread_queue_state_hpp

namespace hemlock {
    namespace thread {
        ////////////////////////////////////////////////////////////////////////////////
        // Thread Tasks

        /**
         * @brief Thread task stub used for type erasure and clean destruction.
         */
        class IThreadTask {
        public:
            virtual ~IThreadTask() {
                // Empty.
            }

            /**
             * @brief Executes the task.
             *
             * @return True if the task completed, false if it needs to
             * be re-queued.
             */
            virtual bool execute() = 0;
        };

        /**
         * @brief A thread task and its ownership status.
         */
        struct QueuedTask {
            IThreadTask* task;
            bool         delete_on_complete;
        };

        ////////////////////////////////////////////////////////////////////////////////
        // Thread Task Queues

        /**
         * @brief Representative type for time duration.
         */
        using TimingRep = std::chrono::nanoseconds;

        /**
         * @brief Basic task queue, a wrapper of moodycamel::BlockingConcurrentQueue
         * that adds tracking of timing and construction of a simple control block for
         * threads.
         */
        class BasicTaskQueue {
        public:
            class ControlBlock {
            public:
                ControlBlock() : m_producer{}, m_consumer{} { }

                ControlBlock(BasicTaskQueue* queue) {
                    new (reinterpret_cast<moodycamel::ProducerToken*>(m_producer))
                        moodycamel::ProducerToken{ queue->m_queue };
                    new (reinterpret_cast<moodycamel::ConsumerToken*>(m_consumer))
                        moodycamel::ConsumerToken{ queue->m_queue };
                };

                inline moodycamel::ProducerToken& producer() {
                    return *reinterpret_cast<moodycamel::ProducerToken*>(m_producer);
                }

                inline moodycamel::ConsumerToken& consumer() {
                    return *reinterpret_cast<moodycamel::ConsumerToken*>(m_consumer);
                }
            protected:
                ui8 m_producer[sizeof(moodycamel::ProducerToken)];
                ui8 m_consumer[sizeof(moodycamel::ConsumerToken)];
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

            bool enqueue(const QueuedTask& item, void* control_block) {
                if (control_block) {
                    return m_queue.enqueue(
                        reinterpret_cast<ControlBlock*>(control_block)->producer(), item
                    );
                } else {
                    return m_queue.enqueue(item);
                }
            }

            bool enqueue(QueuedTask&& item, void* control_block) {
                if (control_block) {
                    return m_queue.enqueue(
                        reinterpret_cast<ControlBlock*>(control_block)->producer(),
                        std::move(item)
                    );
                } else {
                    return m_queue.enqueue(std::move(item));
                }
            }

            bool dequeue(
                QueuedTask*      item,
                TimingRep        timeout,
                BasicTaskQueue** queue,
                void*            control_block
            ) {
                *queue = this;

                if (control_block) {
                    return m_queue.wait_dequeue_timed(
                        reinterpret_cast<ControlBlock*>(control_block)->consumer(),
                        *item,
                        timeout
                    );
                } else {
                    return m_queue.wait_dequeue_timed(*item, timeout);
                }
            }

            void register_timing(TimingRep timing) { m_timings.emplace_back(timing); }
        protected:
            moodycamel::BlockingConcurrentQueue<QueuedTask> m_queue;
            hmem::StackAllocRingBuffer<TimingRep, 10>       m_timings;
        };

        bool dequeue(
            BasicTaskQueue&      queue,
            OUT QueuedTask*      item,
            TimingRep            timeout,
            OUT BasicTaskQueue** queue_out,
            void*                control_block
        );

        /**
         * @brief Defines the requirements on a satisfactory task queue type.
         *
         * NOTE: it is required that any compound queue type be constructed out of some
         * number of the BasicTaskQueue type. In order, therefore, to benefit from the
         * optimisation that comes with possessing a thread-specific consumer and
         * producer token, a control block may be constructed per-thread and passed in
         * to dequeue. This type must be specified by the respective task queue type and
         * so for simplicity we erase its type.
         *
         * @tparam Candidate The candidate typename for being a valid task queue type.
         */
        template <typename Candidate>
        concept IsTaskQueue
            = requires { typename Candidate::ControlBlock; }
              && requires (
                  Candidate                         candidate,
                  size_t                            count,
                  QueuedTask*                       item,
                  typename Candidate::ControlBlock* control_block,
                  TimingRep                         timeout,
                  BasicTaskQueue**                  queue
              ) {
                     {
                         candidate.register_thread()
                     } -> std::same_as<typename Candidate::ControlBlock*>;
                     {
                         candidate.register_threads(count)
                     } -> std::same_as<typename Candidate::ControlBlock*>;
                     {
                         candidate.dequeue(item, timeout, queue, control_block)
                     } -> std::same_as<bool>;
                 };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_state_hpp

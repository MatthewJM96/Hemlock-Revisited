#ifndef __hemlock_thread_queue_state_hpp
#define __hemlock_thread_queue_state_hpp

#include "thread/state.hpp"

namespace hemlock {
    namespace thread {
        /**
         * @brief Representative type for time duration.
         */
        using TimingRep = std::chrono::nanoseconds;

        /**
         * @brief Basic task queue, a wrapper of moodycamel::BlockingConcurrentQueue
         * that adds tracking of timing and construction of a simple control block for
         * threads.
         */
        class BasicTaskQueue : private moodycamel::BlockingConcurrentQueue<QueuedTask> {
            struct ControlBlock {
                ControlBlock(moodycamel::BlockingConcurrentQueue<QueuedTask>* queue) :
                    producer(moodycamel::ProducerToken(*queue)),
                    consumer(moodycamel::ConsumerToken(*queue)){
                        // Empty.
                    };

                moodycamel::ProducerToken producer;
                moodycamel::ConsumerToken consumer;
            };
        public:
            void* register_thread() { return new ControlBlock{ this }; }

            void* register_threads(size_t count) {
                // ProducerToken and ConsumerToken have no default constructor.

                void*         mem            = new ui8[count * sizeof(ControlBlock)];
                ControlBlock* control_blocks = reinterpret_cast<ControlBlock*>(mem);

                for (size_t idx = 0; idx < count; ++idx) {
                    control_blocks[idx] = ControlBlock{ this };
                }

                return control_blocks;
            }

            bool enqueue(const QueuedTask& item, void* control_block) {
                auto self = reinterpret_cast<
                    moodycamel::BlockingConcurrentQueue<QueuedTask>*>(this);

                if (control_block) {
                    return self->enqueue(
                        reinterpret_cast<ControlBlock*>(control_block)->producer, item
                    );
                } else {
                    return self->enqueue(item);
                }
            }

            bool enqueue(QueuedTask&& item, void* control_block) {
                auto self = reinterpret_cast<
                    moodycamel::BlockingConcurrentQueue<QueuedTask>*>(this);

                if (control_block) {
                    return self->enqueue(
                        reinterpret_cast<ControlBlock*>(control_block)->producer,
                        std::move(item)
                    );
                } else {
                    return self->enqueue(std::move(item));
                }
            }

            bool dequeue(
                QueuedTask*      item,
                TimingRep        timeout,
                BasicTaskQueue** queue,
                void*            control_block
            ) {
                auto self = reinterpret_cast<
                    moodycamel::BlockingConcurrentQueue<QueuedTask>*>(this);

                *queue = this;

                if (control_block) {
                    return self->wait_dequeue_timed(
                        reinterpret_cast<ControlBlock*>(control_block)->consumer,
                        *item,
                        timeout
                    );
                } else {
                    return self->wait_dequeue_timed(*item, timeout);
                }
            }

            void register_timing(TimingRep timing) { m_timings.emplace_back(timing); }
        protected:
            hmem::StackAllocRingBuffer<TimingRep, 10> m_timings;
        };

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
            = requires (
                Candidate        candidate,
                QueuedTask*      item,
                TimingRep        timeout,
                BasicTaskQueue** queue,
                void*            control_block
            ) {
                  {
                      candidate.register_thread()
                      } -> std::same_as<void*>;
                  {
                      candidate.enqueue(*item, control_block)
                      } -> std::same_as<bool>;
                  {
                      candidate.dequeue(item, timeout, queue, control_block)
                      } -> std::same_as<bool>;
              };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_state_hpp

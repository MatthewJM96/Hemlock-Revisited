#ifndef __hemlock_thread_queue_basic_hpp
#define __hemlock_thread_queue_basic_hpp

#include "memory/ring_buffer/stack_allocated.hpp"
#include "thread/state.hpp"

namespace hemlock {
    namespace thread {
        class BasicTaskQueue : public moodycamel::BlockingConcurrentQueue<QueuedTask> {
        public:
            QueueDelegate dequeue(QueuedTask& task, std::chrono::microseconds timeout) {
                wait_dequeue_timed(task, timeout);

                return QueueDelegate{ [this](QueuedTask&& task) {
                    return this->enqueue(std::forward<QueuedTask>(task));
                } };
            }
        };

        class TimedBasicTaskQueue : public moodycamel::BlockingConcurrentQueue<QueuedTask> {
        public:
            DequeueDelegates dequeue(QueuedTask& task, std::chrono::microseconds timeout) {
                wait_dequeue_timed(task, timeout);

                return DequeueDelegates{ QueueDelegate{[this](QueuedTask&& task) {
                    return this->enqueue(std::forward<QueuedTask>(task));
                } }, RegisterTimingDelegate{[this](bool, TimingRep timing) {
                    m_timings.push_back(timing);
                }} };
            }
        protected:
            hmem::StackAllocRingBuffer<TimingRep, 10> m_timings;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_basic_hpp

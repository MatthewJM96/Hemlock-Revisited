#ifndef __hemlock_thread_queue_basic_hpp
#define __hemlock_thread_queue_basic_hpp

#include "memory/ring_buffer/stack_allocated.hpp"
#include "thread/queue/state.hpp"
#include "thread/state.hpp"

namespace hemlock {
    namespace thread {
        class BasicTaskQueue : public moodycamel::BlockingConcurrentQueue<QueuedTask> {
        public:
            bool dequeue(QueuedTask& task, TimingRep timeout, void*&) {
                return wait_dequeue_timed(task, timeout);
            }

            bool queue(QueuedTask task, void*) { }

            void register_timing(bool task_succeeded, TimingRep timing, void*) { }
        protected:
            hmem::StackAllocRingBuffer<TimingRep, 10> m_timings;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_basic_hpp

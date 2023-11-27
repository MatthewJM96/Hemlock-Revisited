#ifndef __hemlock_thread_queue_basic_hpp
#define __hemlock_thread_queue_basic_hpp

#include "basic_concepts.hpp"
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
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_basic_hpp

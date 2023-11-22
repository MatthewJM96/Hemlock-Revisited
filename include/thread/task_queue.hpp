#ifndef __hemlock_thread_task_queue_hpp
#define __hemlock_thread_task_queue_hpp

#include "basic_concepts.hpp"

namespace hemlock {
    namespace thread {
        class IThreadTask {
        public:
            virtual ~IThreadTask() {
                // Empty.
            }
        };

        struct QueuedTask {
            IThreadTask* task;
            bool         delete_on_complete;
        };

        using QueueDelegate = Delegate<bool(QueuedTask&&)>;

        class BasicTaskQueue : public moodycamel::BlockingConcurrentQueue<QueuedTask> {
        public:
            QueueDelegate dequeue(QueuedTask& task, std::chrono::microseconds timeout) {
                wait_dequeue_timed(task, timeout);

                return QueueDelegate{ [this](QueuedTask&& task) {
                    return this->enqueue(std::forward<QueuedTask>(task));
                } };
            }
        };

        template <typename Candidate>
        concept IsTaskQueue
            = requires (Candidate c, QueuedTask& i, std::chrono::microseconds t) {
                  {
                      c.dequeue(i, t)
                      } -> std::same_as<QueueDelegate>;
              };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_task_queue_hpp

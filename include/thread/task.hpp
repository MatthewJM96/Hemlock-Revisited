#ifndef __hemlock_thread_task_hpp
#define __hemlock_thread_task_hpp

#include "thread/task_queue.hpp"

namespace hemlock {
    namespace thread {
        class ThreadTaskBase : public IThreadTask {
        public:
            ThreadTaskBase() {
                // Empty.
            }

            virtual ~ThreadTaskBase() {
                // Empty.
            }

            /**
             * @brief If any special handling is needed to clean
             * up task, override this.
             */
            virtual void dispose() {
                // Empty.
            }

            /**
             * @brief Executes the task, this must be implemented
             * by inheriting tasks.
             *
             * @param queue_task Delegate that when called will queue the
             * passed task.
             * @return True if the task completed, false if it needs to
             * be re-queued.
             */
            virtual bool execute(QueueDelegate* queue_task) = 0;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_task_hpp

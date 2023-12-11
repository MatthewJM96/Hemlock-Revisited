#ifndef __hemlock_thread_state_hpp
#define __hemlock_thread_state_hpp

namespace hemlock {
    namespace thread {
        ////////////////////////////////////////////////////////////////////////////////
        // Threads

        /**
         * @brief Essential per-thread state, available to their main functions for
         * handling state changes such as suspension and joining.
         */
        struct ThreadState {
            volatile bool stop;
            volatile bool suspend;
        };

        /**
         * @brief Threads in our threadpools are implemented using std::thread along
         * with some per-thread state as described in ThreadState.
         */
        struct Thread {
            std::thread thread;

            ThreadState state;
        };

        /**
         * @brief Container of threads in a pool.
         */
        using Threads = std::vector<Thread>;

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
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_state_hpp

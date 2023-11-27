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
         * @brief Delegate for queueing a task.
         *
         * @param task The task to queue.
         *
         * @return True if the task was queued, false otherwise.
         */
        using QueueDelegate = Delegate<bool(QueuedTask&&)>;

        /**
         * @brief Defines the requirements on a satisfactory task queue type.
         *
         * NOTE: this is specifically different to the moodycamel API as it is expected
         * that many task queues will simply be an extension of that queue type
         * returning a simple functor that queues a task onto itself.
         * TODO(Matthew): should we modify this to be more performant in the common
         *                case?
         *
         * @tparam Candidate The candidate typename for being a valid task queue type.
         */
        template <typename Candidate>
        concept IsTaskQueue
            = requires (Candidate c, QueuedTask& i, std::chrono::microseconds t) {
                  {
                      c.dequeue(i, t)
                      } -> std::same_as<QueueDelegate>;
              };

        ////////////////////////////////////////////////////////////////////////////////
        // Threadpool

        /**
         * @brief Describes the resolution of timing that should be tracked inside a
         * threadpool.
         *
         * NOTE: this is in order to implement prioritisation and fair sharing using an
         * algorithm such as EEVDF.
         * TODO(Matthew): do we even need this? I suspect we only ever want to do timing
         *                at task resolution or else not at all.
         */
        enum class ThreadpoolTimingResolution {
            NONE,
            ON_SUSPEND,
            ON_TASK_COMPLETION
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_state_hpp

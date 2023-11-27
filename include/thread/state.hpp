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
         * @brief Delegate for registering with a task queue the time to execute the
         * associated task.
         *
         * @param timing The time taken to execute the task.
         */
        using RegisterTimingDelegate = Delegate<void(bool, std::chrono::system_clock::rep)>;

        /**
         * @brief Wrapper for the delergates returned by a task queue's dequeue
         * function.
         */
        using DequeueDelegates = std::tuple<QueueDelegate, RegisterTimingDelegate>;

        // TODO(Matthew): how we've done this is fine for now, but maybe we want to
        //                do an optimisation pass in general. Namely the default of a
        //                light wrapper over moodycamel::BlockingConcurrentQueue is
        //                easily optimised with a specific thread pool implementation.
        //                Likewise timing currently is presumed except up to whether
        //                a thread's main function does anything with it. We probably
        //                should avoid this and do something more sensible there too.

        /**
         * @brief Defines the requirements on a satisfactory task queue type.
         *
         * NOTE: this is specifically different to the moodycamel API as it is expected
         * that many task queues will simply be an extension of that queue type
         * returning a simple functor that queues a task onto itself.
         *
         * @tparam Candidate The candidate typename for being a valid task queue type.
         */
        template <typename Candidate>
        concept IsTaskQueue
            = requires (Candidate c, QueuedTask& i, std::chrono::microseconds t) {
                  {
                      c.dequeue(i, t)
                      } -> std::same_as<DequeueDelegates>;
              };

        ////////////////////////////////////////////////////////////////////////////////
        // Threadpool

        /**
         * @brief Delegate definition for any valid thread main function.
         *
         * @tparam TaskQueue A valid task queue type that the threadpool dequeues tasks
         * from.
         * @tparam Timing Switch for enabling timing.
         */
        template <IsTaskQueue TaskQueue, bool Timing>
        using ThreadMainFunc = Delegate<void(ThreadState*, TaskQueue*)>;
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_state_hpp

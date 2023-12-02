#ifndef __hemlock_thread_queue_state_hpp
#define __hemlock_thread_queue_state_hpp

#include "thread/state.hpp"

namespace hemlock {
    namespace thread {
        // TODO(Matthew): might still need these to be Delegates as we need to be able
        //                to maintian some state with the function pointer to correctly
        //                requeue. even at basic queue level, a function pointer forgets
        //                object.

        /**
         * @brief Delegate for queueing a task.
         *
         * @param task The task to queue.
         *
         * @return True if the task was queued, false otherwise.
         */
        using QueueDelegate = Delegate<bool(QueuedTask&&)>;

        /**
         * @brief Representative type for time duration.
         */
        using TimingRep = std::chrono::system_clock::rep;

        /**
         * @brief Delegate for registering with a task queue the time to execute the
         * associated task.
         *
         * @param timing The time taken to execute the task.
         */
        using RegisterTimingDelegate = Delegate<void(bool, TimingRep)>;

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
                      } -> std::same_as<QueueDelegate>;
              };

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
        concept IsTimedTaskQueue
            = requires (Candidate c, QueuedTask& i, std::chrono::microseconds t) {
                  {
                      c.dequeue(i, t)
                      } -> std::same_as<DequeueDelegates>;
              };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_state_hpp

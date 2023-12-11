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
            = requires (Candidate c, QueuedTask i, TimingRep t, void* u, bool s) {
                  typename Candidate::IdentifierType;
                  {
                      c.dequeue(i, t, u)
                      } -> std::same_as<bool>;
                  {
                      c.queue(i, u)
                      } -> std::same_as<bool>;
                  {
                      c.register_timing(s, t, u)
                      } -> std::same_as<void>;
              };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_queue_state_hpp

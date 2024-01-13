#ifndef __hemlock_thread_state_hpp
#define __hemlock_thread_state_hpp

#include "thread/queue/state.hpp"

namespace hemlock {
    namespace thread {
        ////////////////////////////////////////////////////////////////////////////////
        // Threads

        /**
         * @brief Essential per-thread state, available to their main functions for
         * handling state changes such as suspension and joining.
         */
        template <IsTaskQueue TaskQueue>
        struct ThreadState {
            volatile bool stop;
            volatile bool suspend;

            typename TaskQueue::ControlBlock* queue_control_block;
        };

        /**
         * @brief Threads in our threadpools are implemented using std::thread along
         * with some per-thread state as described in ThreadState.
         */
        template <IsTaskQueue TaskQueue>
        struct Thread {
            std::thread thread;

            ThreadState<TaskQueue> state;
        };

        /**
         * @brief Container of threads in a pool.
         */
        template <IsTaskQueue TaskQueue>
        using Threads = std::vector<Thread<TaskQueue>>;
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_state_hpp

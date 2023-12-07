#ifndef __hemlock_thread_thread_pool_state_hpp
#define __hemlock_thread_thread_pool_state_hpp

#include "thread/queue/state.hpp"

namespace hemlock {
    namespace thread {
        /**
         * @brief Delegate definition for any valid thread main function.
         *
         * @tparam TaskQueue A valid task queue type that the threadpool dequeues tasks
         * from.
         * @tparam Timing Switch for enabling timing.
         */
        template <IsTaskQueue TaskQueue>
        using ThreadMainFunc = Delegate<void(ThreadState*, TaskQueue*)>;
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_thread_pool_state_hpp

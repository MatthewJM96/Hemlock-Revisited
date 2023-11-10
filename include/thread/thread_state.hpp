#ifndef __hemlock_thread_thread_state_hpp
#define __hemlock_thread_thread_state_hpp

namespace hemlock {
    namespace thread {
        template <typename Candidate>
        concept IsThreadState = requires (Candidate state) {
                                    std::is_same_v<decltype(state.stop), bool>;
                                    std::is_same_v<decltype(state.suspend), bool>;
                                };

        struct BasicThreadState {
            volatile bool stop;
            volatile bool suspend;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_thread_state_hpp

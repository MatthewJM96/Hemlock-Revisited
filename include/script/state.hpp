#ifndef __hemlock_script_state_hpp
#define __hemlock_script_state_hpp

#include "call_state.hpp"

namespace hemlock {
    namespace script {
        template <typename ReturnType, typename... Parameters>
        using ScriptDelegate = Delegate<std::conditional_t<
            std::is_void_v<ReturnType>,
            bool(Parameters...),
            std::tuple<bool, ReturnType>(Parameters...)>>;
    }
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_state_hpp

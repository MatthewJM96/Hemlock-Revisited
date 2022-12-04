#ifndef __hemlock_script_call_state_hpp
#define __hemlock_script_call_state_hpp

#include "script/environment_base_decl.hpp"

namespace hemlock {
    namespace script {
        using CallID = i64;

        using Calls = std::vector<std::pair<CallID, std::string>>;

        enum class CallState : ui8 {
            COMPLETE = 0,
            RUNNING  = 1,
            PENDING  = 2
        };

        struct NilType { /* Empty. */
        };

        // TODO(Matthew): Can we support arrays? Tables?
        enum class CallType {
            NIL,
            BOOLEAN,
            NUMBER,
            INTEGER,
            STRING,
            POINTER
        };
        using CallValue = std::variant<NilType, bool, f64, const char*, void*>;

        struct CallParameter {
            CallType  type;
            CallValue value;
        };

        using CallParameters = std::vector<CallParameter>;

        struct CallData {
            i32            index;
            CallState      state;
            CallParameters call_values;
        };

        using CallsData = std::unordered_map<CallID, CallData>;
    }  // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_call_state_hpp

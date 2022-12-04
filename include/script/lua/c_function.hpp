#ifndef __hemlock_script_lua_c_function_hpp
#define __hemlock_script_lua_c_function_hpp

#include "script/lua/state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            namespace impl {
                template <typename ReturnType, typename... Parameters>
                i32 handle_delegate_invocation(
                    LuaHandle state, Delegate<ReturnType, Parameters...>* delegate
                );

                template <typename ReturnType, typename... Parameters>
                i32 handle_function_invocation(
                    LuaHandle state, ReturnType (*func)(Parameters...)
                );

                template <
                    typename Closure,
                    typename ReturnType,
                    typename... Parameters>
                i32 handle_closure_invocation(
                    LuaHandle state,
                    Closure*  closure,
                    ReturnType (Closure::*func)(Parameters...)
                );
            };  // namespace impl

            template <typename ReturnType, typename... Parameters>
            i32 invoke_delegate(LuaHandle state);

            template <typename ReturnType, typename... Parameters>
            i32 invoke_function(LuaHandle state);

            template <typename Closure, typename ReturnType, typename... Parameters>
            i32 invoke_closure(LuaHandle state);
        }  // namespace lua
    }      // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#include "c_function.inl"

#endif  // __hemlock_script_lua_c_function_hpp

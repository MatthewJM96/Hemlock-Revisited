#ifndef __hemlock_script_lua_lua_function_hpp
#define __hemlock_script_lua_lua_function_hpp

#include "script/call_state.hpp"
#include "script/lua/state.hpp"

#if !defined(H_MAX_ARBITRARY_LUA_RETURNS)
#  define HEMLOCK_MAX_ARBITRARY_LUA_RETURNS 10
#endif  // defined(H_MAX_ARBITRARY_LUA_RETURNS)

namespace hemlock {
    namespace script {
        namespace lua {
            /**
             * @brief Function that can be exposed to Lua environment
             * in order for Lua scripts to register their functions
             * with the C++ side.
             *
             * @param state The Lua state.
             * @tparam HasRPCManager Whether the environment has a command buffer.
             * @tparam CallBufferSize The size of the command buffer.
             * @return i32 The number of returned parameters to
             * Lua (will be one, an integer determining success).
             */
            template <bool HasRPCManager, size_t CallBufferSize>
            i32 register_lua_function(LuaHandle state);

            /**
             * @brief Deregisters the given lua function, removing it
             * from the environment's registry table.
             *
             * @param state The Lua state to deregister within.
             * @param lua_func_state The Lua function to deregister.
             */
            void
            deregister_lua_function(LuaHandle state, LuaFunctionState lua_func_state);

            /**
             * @brief Creates a delegate for calling the given
             * Lua function, handling errors and so value passing.
             *
             * Note that the delegate itself returns a tuple of a
             * bool and the ReturnType specified, the bool indicating
             * if the Lua function was successfully called.
             *
             * @tparam ReturnType The return type of the Lua function.
             * @tparam Parameters The parameter types to pass to the Lua
             * function.
             * @param lua_func_state The Lua function.
             * @return ScriptDelegate<ReturnType, Parameters...> The
             * delegate providing call access to the Lua function.
             */
            template <typename ReturnType, typename... Parameters>
            ScriptDelegate<ReturnType, Parameters...>
            make_lua_delegate(LuaFunctionState lua_func_state);

            // TODO(Matthew): figure out the actual signature, will require some
            // parameter
            //                to be passed and something returned that contain the
            //                scalars on both ends.
            /**
             * @brief Creates a delegate for calling the given
             * Lua function with arbitrary scalar parameters and
             * returns, handling errors and value passing.
             *
             * Note that the delegate itself returns a bool indicating
             * if the Lua function was successfully called.
             *
             * @param lua_func_state The Lua function.
             * @return ScriptDelegate<void> The
             * delegate providing call access to the Lua function.
             */
            ScriptDelegate<CallParameters, CallParameters>
            make_arbitrary_scalars_lua_delegate(
                LuaFunctionState lua_func_state, i32 expected_return_count
            );
        }  // namespace lua
    }      // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#include "lua_function.inl"

#endif  // __hemlock_script_lua_lua_function_hpp

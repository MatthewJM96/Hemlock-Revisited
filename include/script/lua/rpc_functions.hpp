#ifndef __hemlock_script_lua_rpc_functions_hpp
#define __hemlock_script_lua_rpc_functions_hpp

#include "state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            /**
             * @brief Lua callable for buffering a call to a foreign
             * function.
             *
             * @param state The Lua state to operate within.
             * @tparam HasRPCManager Whether the environment has an RPC manager.
             * @tparam CallBufferSize The size of the command buffer.
             * @return i32 The number of returned arguments.
             */
            template <bool HasRPCManager, size_t CallBufferSize>
            i32 call_foreign(LuaHandle state);

            /**
             * @brief Lua callable for obtaining the state of
             * a call to a foreign function.
             *
             * @param state The Lua state to operate within.
             * @tparam HasRPCManager Whether the environment has an RPC manager.
             * @tparam CallBufferSize The size of the command buffer.
             * @return i32 The number of returned arguments.
             */
            template <bool HasRPCManager, size_t CallBufferSize>
            i32 query_foreign_call(LuaHandle state);

            /**
             * @brief Lua callable for obtaining the returned
             * values of a call to a foreign function.
             *
             * @param state The Lua state to operate within.
             * @tparam HasRPCManager Whether the environment has an RPC manager.
             * @tparam CallBufferSize The size of the command buffer.
             * @return i32 The number of returned arguments.
             */
            template <bool HasRPCManager, size_t CallBufferSize>
            i32 get_foreign_call_results(LuaHandle state);

            /**
             * @brief Lua callable for setting the state
             * of whether to pump the command buffer
             * manually or automatically.
             *
             * @param state The Lua state to operate within.
             * @tparam HasRPCManager Whether the environment has an RPC manager.
             * @tparam CallBufferSize The size of the command buffer.
             * @return i32 The number of returned arguments.
             */
            template <bool HasRPCManager, size_t CallBufferSize>
            i32 set_manual_command_buffer_pump(LuaHandle state);

            /**
             * @brief Lua callable for beginning the pump
             * of the command buffer. Note that this only
             * does anything if manual command buffer
             * pumping is turned on.
             *
             * @param state The Lua state to operate within.
             * @tparam HasRPCManager Whether the environment has an RPC manager.
             * @tparam CallBufferSize The size of the command buffer.
             * @return i32 The number of returned arguments.
             */
            template <bool HasRPCManager, size_t CallBufferSize>
            i32 pump_command_buffer(LuaHandle state);
        }  // namespace lua
    }      // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#include "rpc_functions.inl"

#endif  // __hemlock_script_lua_rpc_functions_hpp

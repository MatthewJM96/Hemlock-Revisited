#ifndef __hemlock_script_lua_command_buffer_hpp
#define __hemlock_script_lua_command_buffer_hpp

#include "script/command_buffer.hpp"

#include "state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            /**
             * @brief Lua callable for buffering a call to a foreign
             * function.
             *
             * @param state The Lua state to operate within.
             * @tparam HasCommandBuffer Whether the environment has a command buffer.
             * @tparam CommandBufferSize The size of the command buffer.
             * @return i32 The number of returned arguments.
             */
            template <bool HasCommandBuffer, size_t CommandBufferSize>
            i32 call_foreign(LuaHandle state);

            /**
             * @brief Lua callable for obtaining the state of
             * a call to a foreign function.
             *
             * @param state The Lua state to operate within.
             * @tparam HasCommandBuffer Whether the environment has a command buffer.
             * @tparam CommandBufferSize The size of the command buffer.
             * @return i32 The number of returned arguments.
             */
            template <bool HasCommandBuffer, size_t CommandBufferSize>
            i32 query_foreign_call(LuaHandle state);

            /**
             * @brief Lua callable for obtaining the returned
             * values of a call to a foreign function.
             *
             * @param state The Lua state to operate within.
             * @tparam HasCommandBuffer Whether the environment has a command buffer.
             * @tparam CommandBufferSize The size of the command buffer.
             * @return i32 The number of returned arguments.
             */
            template <bool HasCommandBuffer, size_t CommandBufferSize>
            i32 get_foreign_call_results(LuaHandle state);

            /**
             * @brief Lua callable for setting the state
             * of whether to pump the command buffer
             * manually or automatically.
             *
             * @param state The Lua state to operate within.
             * @tparam HasCommandBuffer Whether the environment has a command buffer.
             * @tparam CommandBufferSize The size of the command buffer.
             * @return i32 The number of returned arguments.
             */
            template <bool HasCommandBuffer, size_t CommandBufferSize>
            i32 set_manual_command_buffer_pump(LuaHandle state);

            /**
             * @brief Lua callable for beginning the pump
             * of the command buffer. Note that this only
             * does anything if manual command buffer
             * pumping is turned on.
             *
             * @param state The Lua state to operate within.
             * @tparam HasCommandBuffer Whether the environment has a command buffer.
             * @tparam CommandBufferSize The size of the command buffer.
             * @return i32 The number of returned arguments.
             */
            template <bool HasCommandBuffer, size_t CommandBufferSize>
            i32 pump_command_buffer(LuaHandle state);
        }
    }
}
namespace hscript = hemlock::script;

#include "command_buffer.inl"

#endif // __hemlock_script_lua_command_buffer_hpp

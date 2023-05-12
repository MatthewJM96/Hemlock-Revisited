#ifndef __hemlock_script_lua_state_hpp
#define __hemlock_script_lua_state_hpp

#include "script/state.hpp"

#define HEMLOCK_LUA_SCRIPT_FUNCTION_TABLE             "ScriptFunctions"
#define HEMLOCK_LUA_CONTINUABLE_SCRIPT_FUNCTION_TABLE "ContinuableScriptFunctions"

namespace hemlock {
    namespace script {
        namespace lua {
            using LuaHandle    = lua_State*;
            using LuaInvocable = int (*)(lua_State*);

            struct LuaFunctionState {
                LuaHandle state;
                i32       index;
            };

            /**
             * @brief Provides an API for moving data between Lua stack and C++ side.
             * It is implemented for scalar types, pointer types, bounded array types
             * and glm vectors.
             *
             * @tparam The type of the data to be moved.
             */
            template <typename, typename = void>
            struct LuaValue { };

            template <bool HasRPCManager, size_t CallBufferSize>
            class Environment;
        }  // namespace lua
    }      // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_lua_state_hpp

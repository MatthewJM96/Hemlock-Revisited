#ifndef __hemlock_script_lua_state_hpp
#define __hemlock_script_lua_state_hpp

#include "script/state.hpp"

#define HEMLOCK_LUA_SCRIPT_FUNCTION_TABLE "ScriptFunctions"

namespace hemlock {
    namespace script {
        namespace lua {
            using LuaHandle    = lua_State*;
            using LuaInvocable = int (*)(lua_State*);

            struct LuaFunctionState {
                LuaHandle state;
                i32       index;
            };
        }  // namespace lua
    }      // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_lua_state_hpp

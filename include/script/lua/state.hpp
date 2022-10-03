#ifndef __hemlock_script_lua_state_hpp
#define __hemlock_script_lua_state_hpp

#include "script/state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            using LuaHandle     = lua_State*;
            using LuaInvocable  = int(*)(lua_State*);

            struct LuaFunctionState {
                LuaHandle   state;
                std::string name;
                i32         index;
            };
        }
    }
}
namespace hscript = hemlock::script;

#endif // __hemlock_script_lua_state_hpp

#ifndef __hemlock_script_lua_state_hpp
#define __hemlock_script_lua_state_hpp

namespace hemlock {
    namespace script {
        namespace lua {
            using LuaHandle     = lua_State*;
            using LuaInvocable  = int(*)(lua_State*);
        }
    }
}
namespace hscript = hemlock::script;

#endif // __hemlock_script_lua_state_hpp

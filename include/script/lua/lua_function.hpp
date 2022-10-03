#ifndef __hemlock_script_lua_lua_function_hpp
#define __hemlock_script_lua_lua_function_hpp

#include "script/lua/state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
                /**
                 * @brief Function that can be exposed to Lua environment
                 * in order for Lua scripts to register their functions
                 * with the C++ side.
                 *
                 * @param state The Lua state.
                 * @return i32 The number of returned parameters to
                 * Lua (will be one, an integer determining success).
                 */
                i32 register_lua_function(LuaHandle state);

        }
    }
}
namespace hscript = hemlock::script;

#endif // __hemlock_script_lua_lua_function_hpp

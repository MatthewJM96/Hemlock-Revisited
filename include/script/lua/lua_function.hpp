#ifndef __hemlock_script_lua_lua_function_hpp
#define __hemlock_script_lua_lua_function_hpp

#include "script/lua/state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            struct LuaFunctionState {
                LuaHandle   state;
                std::string name;
                i32         index;
            };


        }
    }
}
namespace hscript = hemlock::script;

#endif // __hemlock_script_lua_lua_function_hpp

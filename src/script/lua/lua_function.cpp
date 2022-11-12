#include "stdafx.h"

#include "script/lua/environment.hpp"

#include "script/lua/lua_function.hpp"

void hscript::lua::deregister_lua_function(LuaHandle state, LuaFunctionState lua_func_state) {
    lua_getfield(state, LUA_REGISTRYINDEX, H_LUA_SCRIPT_FUNCTION_TABLE);
    luaL_unref(state, -1, lua_func_state.index);

    // NOTE(Matthew): Docs don't say if luaL_unref pops the indexed-into
    //                object, and it likely doesn't. We pop here, but if
    //                luaL_unref does in fact pop, then this will cause
    //                issues.
    lua_pop(state, 1);
}

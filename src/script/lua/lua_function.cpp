#include "stdafx.h"

#include "script/lua/environment.hpp"

#include "script/lua/lua_function.hpp"

i32 hscript::lua::register_lua_function(LuaHandle state) {
    // Get the captured environment pointer.
    Environment* env = LuaValue<Environment*>::retrieve_upvalue(state, 1);

    // We expect only one parameter - a string identifying the function to be registered.
    if (lua_gettop(state) != 1) {
        LuaValue<i32>::push(state, -1);
        return 1;
    }

    // Try to get the name of the function, if we can't then return.
    std::string name;
    if (!LuaValue<std::string>::try_pop(state, name)) {
        LuaValue<i32>::push(state, -2);
        return 1;
    }

    // Retrieve the Lua function from the environment, registering
    // it if it has not yet been registered. False return indicates
    // we failed to register the function, report failure.
    if (!env->register_lua_function(name)) {
        LuaValue<i32>::push(state, -3);
        return 1;
    }

    // Lua function registered, report success.
    LuaValue<i32>::push(state, 0);
    return 1;
}

void hscript::lua::deregister_lua_function(LuaHandle state, LuaFunctionState lua_func_state) {
    lua_getfield(state, LUA_REGISTRYINDEX, H_LUA_SCRIPT_FUNCTION_TABLE);
    luaL_unref(state, -1, lua_func_state.index);

    // NOTE(Matthew): Docs don't say if luaL_unref pops the indexed-into
    //                object, and it likely doesn't. We pop here, but if
    //                luaL_unref does in fact pop, then this will cause
    //                issues.
    lua_pop(state, 1);
}

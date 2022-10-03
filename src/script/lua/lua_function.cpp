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

    // First see if the lua function we just got already exists in the environment's register.
    LuaFunctionState* lua_func = env->get_lua_function(name);
    if (lua_func != nullptr) {
        LuaValue<i32>::push(state, 0);
        return 1;
    }

    // If it doesn't, make a registration for it.
    LuaValue<i32>::push(state, env->make_lua_function(name));
    return 1;
}

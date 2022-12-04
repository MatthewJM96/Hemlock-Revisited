#include "stdafx.h"

#include "script/lua/environment.hpp"

#include "script/lua/lua_function.hpp"

void hscript::lua::deregister_lua_function(
    LuaHandle state, LuaFunctionState lua_func_state
) {
    lua_getfield(state, LUA_REGISTRYINDEX, H_LUA_SCRIPT_FUNCTION_TABLE);
    luaL_unref(state, -1, lua_func_state.index);

    // NOTE(Matthew): Docs don't say if luaL_unref pops the indexed-into
    //                object, and it likely doesn't. We pop here, but if
    //                luaL_unref does in fact pop, then this will cause
    //                issues.
    lua_pop(state, 1);
}

hscript::ScriptDelegate<hscript::CallParameters, hscript::CallParameters>
hscript::lua::make_arbitrary_scalars_lua_delegate(
    LuaFunctionState lua_func_state, i32 expected_return_count
) {
    using ReturnType = decltype(ScriptDelegate<CallParameters, CallParameters>(
    )(CallParameters{}));

    return ScriptDelegate<CallParameters, CallParameters>{
        [lua_func_state, expected_return_count](CallParameters parameters) {
            LuaHandle state = lua_func_state.state;

            // Get stack height before preparing & calling the Lua function.
            i32 prior_index = lua_gettop(state);

            // Put the script function table in the lua registry on to the top of the
            // lua stack.
            lua_getfield(state, LUA_REGISTRYINDEX, H_LUA_SCRIPT_FUNCTION_TABLE);
            // Get a raw accessor to the wrapped script function.
            lua_rawgeti(state, -1, lua_func_state.index);

            for (auto& param : parameters) {
                switch (param.type) {
                    case CallType::NIL:
                        lua_pushnil(state);
                        break;
                    case CallType::BOOLEAN:
                        lua_pushboolean(state, std::get<bool>(param.value));
                        break;
                    case CallType::NUMBER:
                        lua_pushnumber(state, std::get<f64>(param.value));
                        break;
                    case CallType::POINTER:
                        lua_pushlightuserdata(state, std::get<void*>(param.value));
                        break;
                    case CallType::STRING:
                        lua_pushstring(state, std::get<const char*>(param.value));
                        break;
                    default:
                        debug_printf(
                            "Lua Delegate with unsupported parameter type: %i",
                            static_cast<i32>(param.type)
                        );
                        lua_pop(state, lua_gettop(state) - prior_index);
                        return ReturnType{ false, {} };
                }
            }

            i32 return_count = expected_return_count < H_MAX_ARBITRARY_LUA_RETURNS ?
                                   expected_return_count :
                                   H_MAX_ARBITRARY_LUA_RETURNS;

            // Call the function with the provided parameters.
            if (lua_pcall(state, parameters.size(), return_count, 0) != 0) {
                debug_printf(
                    "Error calling Lua function: %s.\n",
                    LuaValue<const char*>::pop(state)
                );

                return ReturnType{ false, {} };
            }

            // We sdon't want to fail on unrecognised return type. Caller has control
            // only of their environment, so they can pass Nil for unsupported
            // parameters, but if a function they need to call returns something we
            // can't pass over they simply cannot call it if we fail on this..
            CallParameters return_values(static_cast<size_t>(return_count));
            for (i32 i = -return_count; i < 0; ++i) {
                if (lua_isboolean(state, i)) {
                    return_values[return_count + i]
                        = { .type  = CallType::BOOLEAN,
                            .value = static_cast<bool>(lua_toboolean(state, i)) };
                } else if (lua_isnumber(state, i)) {
                    return_values[return_count + i]
                        = { .type  = CallType::NUMBER,
                            .value = lua_tonumber(state, i) };
                } else if (lua_islightuserdata(state, i)) {
                    return_values[return_count + i]
                        = { .type  = CallType::POINTER,
                            .value = lua_touserdata(state, i) };
                } else if (lua_isstring(state, i)) {
                    return_values[return_count + i]
                        = { .type  = CallType::STRING,
                            .value = lua_tostring(state, i) };
                } else {
                    return_values[return_count + i]
                        = { .type = CallType::NIL, .value = {} };
                }
            }

            // Restore stack to prior state and return.
            lua_pop(state, lua_gettop(state) - prior_index);

            return ReturnType{ true, return_values };
        }
    };
}

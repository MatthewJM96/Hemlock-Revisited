#include "lua_value.hpp"

template <bool HasRPCManager, size_t CallBufferSize>
i32 hscript::lua::register_lua_function(LuaHandle state) {
    using _Environment = Environment<HasRPCManager, CallBufferSize>;

    // Get the captured environment pointer.
    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    // We expect only one parameter - a string identifying the function to be
    // registered.
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
    if (!env->register_lua_function(std::move(name))) {
        LuaValue<i32>::push(state, -3);
        return 1;
    }

    // Lua function registered, report success.
    LuaValue<i32>::push(state, 0);
    return 1;
}

template <typename ReturnType, typename... Parameters>
hscript::ScriptDelegate<ReturnType, Parameters...>
hscript::lua::make_lua_delegate(LuaFunctionState lua_func_state) {
    return ScriptDelegate<ReturnType, Parameters...>{ [lua_func_state](
                                                          Parameters... params
                                                      ) {
        LuaHandle state = lua_func_state.state;

        // Get stack height before preparing & calling the Lua function.
        i32 prior_index = lua_gettop(state);

        // Put the script function table in the lua registry on to the top of the lua
        // stack.
        lua_getfield(state, LUA_REGISTRYINDEX, H_LUA_SCRIPT_FUNCTION_TABLE);
        // Get a raw accessor to the wrapped script function.
        lua_rawgeti(state, -1, lua_func_state.index);

        (LuaValue<Parameters>::push(state, params), ...);

        // Get number of items pushed.
        i32 number_items = static_cast<i32>(total_value_count<Parameters...>());

        // Call the function with the provided parameters.
        if (lua_pcall(state, number_items, LuaValue<ReturnType>::value_count(), 0)
            != 0)
        {
            debug_printf(
                "Error calling Lua function: %s.\n", LuaValue<const char*>::pop(state)
            );

            if constexpr (!std::is_same<ReturnType, void>()) {
                return { false, {} };
            } else {
                return false;
            }
        }

        if constexpr (!std::is_same<ReturnType, void>()) {
            // Try to pop return value from Lua stack, report
            // failure if we can't.
            std::tuple<bool, ReturnType> ret = { true, {} };
            if (!LuaValue<ReturnType>::try_pop(state, ret[1])) {
                debug_printf("Could not obtain return value from Lua function.");

                ret[0] = false;
            }

            // Restore stack to prior state and return
            lua_pop(state, lua_gettop(state) - prior_index);
            return ret;
        } else {
            // Restore stack to prior state and return
            lua_pop(state, lua_gettop(state) - prior_index);
            return true;
        }
    } };
}

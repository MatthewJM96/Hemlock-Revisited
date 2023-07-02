#include "lua_value.hpp"

template <size_t CallBufferSize>
i32 hscript::lua::call_foreign(LuaHandle state) {
    using _Environment = Environment<true, CallBufferSize>;

    // Get the captured environment pointer.
    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    // We expect at least two parameters:
    //   string - environment name
    //   string - function name (namespaced with dot separation)
    //   mixed  - parameters to pass to the foreign function
    if (lua_gettop(state) < 2) {
        lua_settop(state, 0);
        LuaValue<i32>::push(state, -1);
        return 1;
    }

    // Try to get the name of the environment, if we can't then return.
    std::string env_name;
    if (!LuaValue<std::string>::try_retrieve<false>(state, 1, env_name)) {
        lua_settop(state, 0);
        LuaValue<i32>::push(state, -2);
        return 1;
    }

    // Try to get the name of the function, if we can't then return.
    std::string function_name;
    if (!LuaValue<std::string>::try_retrieve<false>(state, 2, function_name)) {
        lua_settop(state, 0);
        LuaValue<i32>::push(state, -3);
        return 1;
    }

    i32            param_count = lua_gettop(state) - 2;
    CallParameters parameters(static_cast<size_t>(param_count));

    for (i32 i = -param_count; i < 0; ++i) {
        if (lua_isboolean(state, i)) {
            parameters[param_count + i]
                = { .type  = CallType::BOOLEAN,
                    .value = static_cast<bool>(lua_toboolean(state, i)) };
        } else if (lua_isnumber(state, i)) {
            parameters[param_count + i]
                = { .type = CallType::NUMBER, .value = lua_tonumber(state, i) };
        } else if (lua_islightuserdata(state, i)) {
            parameters[param_count + i]
                = { .type = CallType::POINTER, .value = lua_touserdata(state, i) };
        } else if (lua_isstring(state, i)) {
            parameters[param_count + i]
                = { .type = CallType::STRING, .value = lua_tostring(state, i) };
        } else {
            parameters[param_count + i] = { .type = CallType::NIL, .value = {} };
        }
    }

    // Get foreign environment.
    _Environment* foreign_env = env->m_registry->get_environment(env_name);

    // Buffer function call in foreign env.
    CallID call_id
        = foreign_env->rpc.append_call(std::move(function_name), std::move(parameters));

    // Pass command ID back to caller.
    lua_settop(state, 0);
    LuaValue<CallID>::push(state, call_id);
    return 1;
}

template <size_t CallBufferSize>
i32 hscript::lua::query_foreign_call(LuaHandle state) {
    using _Environment = Environment<true, CallBufferSize>;

    // Get the captured environment pointer.
    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    // We expect at two parameters:
    //   string - environment name
    //   integer - call ID
    if (lua_gettop(state) != 2) {
        LuaValue<i32>::push(state, -1);
        return 1;
    }

    // Try to get the ID of the command, if we can't then return.
    CallID call_id;
    if (!LuaValue<CallID>::try_pop(state, call_id)) {
        LuaValue<i32>::push(state, -3);
        return 1;
    }

    // Try to get the name of the environment, if we can't then return.
    std::string env_name;
    if (!LuaValue<std::string>::try_pop(state, env_name)) {
        LuaValue<i32>::push(state, -2);
        return 1;
    }

    // Get foreign environment.
    _Environment* foreign_env = env->m_registry->get_environment(env_name);

    // Buffer function call in foreign env.
    CallState cmd_state;
    i32       ret = foreign_env->rpc.call_state(call_id, cmd_state);

    if (ret < 0) {
        // Pass command ID back to caller.
        LuaValue<i32>::push(state, ret);
        return 1;
    }

    LuaValue<CallState>::push(state, cmd_state);
    return 1;
}

template <size_t CallBufferSize>
i32 hscript::lua::get_foreign_call_results(LuaHandle state) {
    using _Environment = Environment<true, CallBufferSize>;

    // Get the captured environment pointer.
    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    // We expect at two parameters:
    //   string - environment name
    //   integer - call ID
    if (lua_gettop(state) != 2) {
        LuaValue<i32>::push(state, -1);
        return 1;
    }

    // Try to get the ID of the command, if we can't then return.
    CallID call_id;
    if (!LuaValue<CallID>::try_pop(state, call_id)) {
        LuaValue<i32>::push(state, -3);
        return 1;
    }

    // Try to get the name of the environment, if we can't then return.
    std::string env_name;
    if (!LuaValue<std::string>::try_pop(state, env_name)) {
        LuaValue<i32>::push(state, -2);
        return 1;
    }

    // Get foreign environment.
    _Environment* foreign_env = env->m_registry->get_environment(env_name);

    // Buffer function call in foreign env.
    CallParameters return_values;
    i32            ret = foreign_env->rpc.call_return_values(call_id, return_values);

    if (ret < 0) {
        // Pass command ID back to caller.
        LuaValue<i32>::push(state, ret);
        return 1;
    }

    for (auto& return_value : return_values) {
        switch (return_value.type) {
            case CallType::BOOLEAN:
                lua_pushboolean(state, std::get<bool>(return_value.value));
                break;
            case CallType::NUMBER:
                lua_pushnumber(state, std::get<f64>(return_value.value));
                break;
            case CallType::POINTER:
                lua_pushlightuserdata(state, std::get<void*>(return_value.value));
                break;
            case CallType::STRING:
                lua_pushstring(state, std::get<const char*>(return_value.value));
                break;
            default:
                lua_pushnil(state);
                break;
        }
    }

    return static_cast<i32>(return_values.size());
}

template <size_t CallBufferSize>
i32 hscript::lua::set_manual_command_buffer_pump(LuaHandle state) {
    using _Environment = Environment<true, CallBufferSize>;

    // Get the captured environment pointer.
    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    // We expect at most one parameter: bool flag indicating if to
    // manual pump.
    if (lua_gettop(state) > 1) {
        LuaValue<i32>::push(state, -1);
        return 1;
    }

    bool manual_pump = true;
    if (lua_gettop(state) == 1) {
        // Try to get the bool flag if it exists, if we can't then return.
        if (!LuaValue<bool>::try_pop(state, manual_pump)) {
            LuaValue<i32>::push(state, -2);
            return 1;
        }
    }

    env->m_rpc_manual_pump = manual_pump;

    LuaValue<i32>::push(state, 0);
    return 1;
}

template <size_t CallBufferSize>
i32 hscript::lua::pump_command_buffer(LuaHandle state) {
    using _Environment = Environment<true, CallBufferSize>;

    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    // env->pump_command_buffer();
    env->rpc.pump_calls();

    return 0;
}

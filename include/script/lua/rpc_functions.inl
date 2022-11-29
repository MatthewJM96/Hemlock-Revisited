template <bool HasRPCManager, size_t CallBufferSize>
i32 hscript::lua::call_foreign(LuaHandle state) {
    using _Environment = Environment<HasRPCManager, CallBufferSize>;

    // Get the captured environment pointer.
    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    // We expect at least two parameters:
    //   string - environment name
    //   string - function name (namespaced with dot separation)
    //   mixed  - parameters to pass to the foreign function
    if (lua_gettop(state) >= 2) {
        LuaValue<i32>::push(state, -1);
        return 1;
    }

    // Try to get the name of the environment, if we can't then return.
    std::string env_name;
    if (!LuaValue<std::string>::try_pop(state, env_name)) {
        LuaValue<i32>::push(state, -2);
        return 1;
    }

    // Try to get the name of the function, if we can't then return.
    std::string function_name;
    if (!LuaValue<std::string>::try_pop(state, function_name)) {
        LuaValue<i32>::push(state, -3);
        return 1;
    }

    // TODO(Matthew): obtain parameters, determining their type
    //                and encoding value as bytes. Command buffer
    //                needs to be extended to support this too.

    // Get foreign environment.
    _Environment* foreign_env = env->m_registry->get_environment(env_name);

    // Buffer function call in foreign env.
    CallID call_id = foreign_env->m_command_buffer.append_command(std::move(function_name));

    // Pass command ID back to caller.
    LuaValue<CallID>::push(state, call_id);
    return 1;
}

template <bool HasRPCManager, size_t CallBufferSize>
i32 hscript::lua::query_foreign_call(LuaHandle state) {
    using _Environment = Environment<HasRPCManager, CallBufferSize>;

    // Get the captured environment pointer.
    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    // We expect one parameter: the command ID.
    if (lua_gettop(state) == 2) {
        LuaValue<i32>::push(state, -1);
        return 1;
    }

    // Try to get the name of the environment, if we can't then return.
    std::string env_name;
    if (!LuaValue<std::string>::try_pop(state, env_name)) {
        LuaValue<i32>::push(state, -2);
        return 1;
    }

    // Try to get the ID of the command, if we can't then return.
    CallID call_id;
    if (!LuaValue<CallID>::try_pop(state, call_id)) {
        LuaValue<i32>::push(state, -3);
        return 1;
    }

    // Get foreign environment.
    _Environment* foreign_env = env->m_registry->get_environment(env_name);

    // Buffer function call in foreign env.
    CallState cmd_state;
    i32 ret = foreign_env->m_command_buffer.command_state(call_id, cmd_state);

    if (ret < 0) {
        // Pass command ID back to caller.
        LuaValue<i32>::push(state, ret);
        return 1;
    }

    LuaValue<CallState>::push(state, cmd_state);
    return 1;
}

template <bool HasRPCManager, size_t CallBufferSize>
i32 hscript::lua::get_foreign_call_results(LuaHandle state) {
    using _Environment = Environment<HasRPCManager, CallBufferSize>;

    // Get the captured environment pointer.
    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    // We expect one parameter: the command ID.
    if (lua_gettop(state) == 2) {
        LuaValue<i32>::push(state, -1);
        return 1;
    }

    // Try to get the name of the environment, if we can't then return.
    std::string env_name;
    if (!LuaValue<std::string>::try_pop(state, env_name)) {
        LuaValue<i32>::push(state, -2);
        return 1;
    }

    // Try to get the ID of the command, if we can't then return.
    CallID call_id;
    if (!LuaValue<CallID>::try_pop(state, call_id)) {
        LuaValue<i32>::push(state, -3);
        return 1;
    }

    // Get foreign environment.
    _Environment* foreign_env = env->m_registry->get_environment(env_name);

    // Buffer function call in foreign env.
    CallParameters return_values;
    i32 ret = foreign_env->m_command_buffer.command_state(call_id, return_values);

    if (ret < 0) {
        // Pass command ID back to caller.
        LuaValue<i32>::push(state, ret);
        return 1;
    }

    // TODO(Matthew): Push return values onto the stack.
    return return_values.size();
}

template <bool HasRPCManager, size_t CallBufferSize>
i32 hscript::lua::set_manual_command_buffer_pump(LuaHandle state) {
    using _Environment = Environment<HasRPCManager, CallBufferSize>;

    // Get the captured environment pointer.
    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    // We expect at most one parameter: bool flag indicating if to
    // manual pump.
    if (lua_gettop(state) <= 1) {
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

    env->m_command_buffer_manual_pump = manual_pump;

    LuaValue<i32>::push(state, 0);
    return 1;
}

template <bool HasRPCManager, size_t CallBufferSize>
i32 hscript::lua::pump_command_buffer(LuaHandle state) {
    using _Environment = Environment<HasRPCManager, CallBufferSize>;

    _Environment* env = LuaValue<_Environment*>::retrieve_upvalue(state, 1);

    env->pump_command_buffer();

    return 0;
}

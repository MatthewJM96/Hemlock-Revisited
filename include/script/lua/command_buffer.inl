template <bool HasCommandBuffer, size_t CommandBufferSize>
i32 hscript::lua::call_foreign(LuaHandle state) {
    using _Environment = Environment<HasCommandBuffer, CommandBufferSize>;

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
    CommandID cmd_id = foreign_env->m_command_buffer.append_command(std::move(function_name));

    // Pass command ID back to caller.
    LuaValue<CommandID>::push(state, cmd_id);
    return 1;
}

template <bool HasCommandBuffer, size_t CommandBufferSize>
i32 hscript::lua::query_foreign_call(LuaHandle state) {
    using _Environment = Environment<HasCommandBuffer, CommandBufferSize>;

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
    CommandID cmd_id;
    if (!LuaValue<CommandID>::try_pop(state, cmd_id)) {
        LuaValue<i32>::push(state, -3);
        return 1;
    }

    // Get foreign environment.
    _Environment* foreign_env = env->m_registry->get_environment(env_name);

    // Buffer function call in foreign env.
    CommandState cmd_state;
    i32 ret = foreign_env->m_command_buffer.command_state(cmd_id, cmd_state);

    if (ret < 0) {
        // Pass command ID back to caller.
        LuaValue<i32>::push(state, ret);
        return 1;
    }

    LuaValue<CommandState>::push(state, cmd_state);
    return 1;
}

template <bool HasCommandBuffer, size_t CommandBufferSize>
i32 hscript::lua::get_foreign_call_results(LuaHandle state) {
    using _Environment = Environment<HasCommandBuffer, CommandBufferSize>;

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
    CommandID cmd_id;
    if (!LuaValue<CommandID>::try_pop(state, cmd_id)) {
        LuaValue<i32>::push(state, -3);
        return 1;
    }

    // Get foreign environment.
    _Environment* foreign_env = env->m_registry->get_environment(env_name);

    // Buffer function call in foreign env.
    CommandReturnValues cmd_return_values;
    i32 ret = foreign_env->m_command_buffer.command_state(cmd_id, cmd_return_values);

    if (ret < 0) {
        // Pass command ID back to caller.
        LuaValue<i32>::push(state, ret);
        return 1;
    }

    // TODO(Matthew): Push return values onto the stack.
    return cmd_return_values.size();
}

template <bool HasCommandBuffer, size_t CommandBufferSize>
i32 hscript::lua::set_manual_command_buffer_pump(LuaHandle state) {

}

template <bool HasCommandBuffer, size_t CommandBufferSize>
i32 hscript::lua::pump_command_buffer(LuaHandle state) {

}
template <
    typename ReturnType,
    typename... NewCallParameters,
    typename... ContinuationParameters>
void hscript::lua::LuaContinuableFunction<
    std::tuple<ReturnType, NewCallParameters...>,
    std::tuple<ReturnType, ContinuationParameters...>>::attach_to_thread(LuaThreadState
                                                                             thread) {
    m_thread            = thread;
    _Base::m_is_yielded = false;

    lua_pop(thread.thread, lua_gettop(thread.thread));

    // Get function registry table.
    lua_getfield(
        m_function.state, LUA_REGISTRYINDEX, HEMLOCK_LUA_SCRIPT_FUNCTION_TABLE
    );

    // Put function onto the script environment stack.
    lua_rawgeti(m_function.state, -1, m_function.index);

    // Move function over to thread.
    lua_xmove(m_function.state, m_thread.thread, 1);
}

template <
    typename ReturnType,
    typename... NewCallParameters,
    typename... ContinuationParameters>
void hscript::lua::LuaContinuableFunction<
    std::tuple<ReturnType, NewCallParameters...>,
    std::tuple<ReturnType, ContinuationParameters...>>::detach_from_thread() {
    if (m_thread.thread == nullptr) return;

    lua_pop(m_thread.thread, lua_gettop(m_thread.thread));

    m_thread            = {};
    _Base::m_is_yielded = false;
}

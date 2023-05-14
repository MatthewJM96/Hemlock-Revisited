#include "script/lua/lua_value.hpp"

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

template <
    typename ReturnType,
    typename... NewCallParameters,
    typename... ContinuationParameters>
hscript::ContinuationResult<ReturnType> hscript::lua::LuaContinuableFunction<
    std::tuple<ReturnType, NewCallParameters...>,
    std::tuple<ReturnType, ContinuationParameters...>>::
    invoke_new_call(NewCallParameters&&... parameters) {
    return do_invocation<NewCallParameters...>(
        std::forward<ContinuationParameters>(parameters)...
    );
}

template <
    typename ReturnType,
    typename... NewCallParameters,
    typename... ContinuationParameters>
hscript::ContinuationResult<ReturnType> hscript::lua::LuaContinuableFunction<
    std::tuple<ReturnType, NewCallParameters...>,
    std::tuple<ReturnType, ContinuationParameters...>>::
    invoke_continuation(ContinuationParameters&&... parameters) {
    return do_invocation<ContinuationParameters...>(
        std::forward<ContinuationParameters>(parameters)...
    );
}

template <
    typename ReturnType,
    typename... NewCallParameters,
    typename... ContinuationParameters>
template <typename... Parameters>
hscript::ContinuationResult<ReturnType> hscript::lua::LuaContinuableFunction<
    std::tuple<ReturnType, NewCallParameters...>,
    std::tuple<ReturnType, ContinuationParameters...>>::
    do_invocation(Parameters&&... parameters) {
    i32 prior_index = lua_gettop(m_thread.thread);

    (LuaValue<NewCallParameters>::push(m_thread.thread, parameters), ...);

    // Get number of items pushed.
    i32 number_items = static_cast<i32>(total_value_count<NewCallParameters...>());

    auto result = lua_resume(m_thread.thread, number_items);

    // Call the function with the provided parameters.
    if (result == 0) {
        _Base::m_is_yielded = false;
    } else if (result == LUA_YIELD) {
        _Base::m_is_yielded = true;
    } else {
        debug_printf(
            "Error calling Lua function: %s.\n", LuaValue<const char*>::pop(state)
        );

        return result;
    }

    if constexpr (!std::is_same<ReturnType, void>()) {
        // Try to pop return value from Lua stack, report
        // failure if we can't.
        ContinuationResult<ReturnType> ret = { result, {} };
        if (!LuaValue<ReturnType>::try_pop(m_thread.thread, ret[1])) {
            debug_printf("Could not obtain return value from Lua function.");

            ret[0] = HEMLOCK_SCRIPT_RETURN_TYPE_ERR;
        }

        // Restore stack to prior state and return
        lua_pop(m_thread.thread, lua_gettop(m_thread.thread) - prior_index);
        return ret;
    } else {
        // Restore stack to prior state and return
        lua_pop(m_thread.thread, lua_gettop(m_thread.thread) - prior_index);
        return result;
    }
}

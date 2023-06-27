#include "script/lua/lua_value.hpp"

namespace hemlock {
    namespace script {
        namespace lua {

            namespace impl {
                template <
                    typename ReturnType,
                    typename... Parameters,
                    typename = typename std::enable_if_t<!std::is_void_v<ReturnType>>>
                i32 handle_delegate_invocation(
                    LuaHandle state, Delegate<ReturnType(Parameters...)>* delegate
                ) {
                    // Parameters and a return value, pop parameters off
                    // Lua stack, call with these, then report number of
                    // items pushed onto Lua stack.

                    ReturnType ret = (*delegate)(LuaValue<Parameters>::pop(state)...);

                    return LuaValue<ReturnType>::push(state, ret);
                }

                template <
                    typename ReturnType,
                    typename = typename std::enable_if_t<!std::is_void_v<ReturnType>>>
                i32 handle_delegate_invocation(
                    LuaHandle state, Delegate<ReturnType>* delegate
                ) {
                    // No parameters but a return value, just call then
                    // report number of items pushed onto Lua stack.

                    ReturnType ret = (*delegate)();

                    return LuaValue<ReturnType>::push(state, ret);
                }

                template <
                    typename ReturnType,
                    typename... Parameters,
                    typename = typename std::enable_if_t<std::is_void_v<ReturnType>>>
                i32 handle_delegate_invocation(
                    LuaHandle state, Delegate<void(Parameters...)>* delegate
                ) {
                    // Parameters but no return value, pop parameters off
                    // Lua stack, call with these, then report zero items
                    // added to Lua stack.

                    (*delegate)(LuaValue<Parameters>::pop(state)...);

                    return 0;
                }

                template <
                    typename ReturnType,
                    typename = typename std::enable_if_t<std::is_void_v<ReturnType>>>
                i32 handle_delegate_invocation(LuaHandle, Delegate<void()>* delegate) {
                    // No parameters, no return value, just call and
                    // report zero items added to Lua stack.

                    (*delegate)();
                    return 0;
                }

                template <
                    typename ReturnType,
                    typename... Parameters,
                    typename = typename std::enable_if_t<!std::is_void_v<ReturnType>>>
                i32 handle_function_invocation(
                    LuaHandle state, ReturnType (*func)(Parameters...)
                ) {
                    // Parameters and a return value, pop parameters off
                    // Lua stack, call with these, then report number of
                    // items pushed onto Lua stack.

                    ReturnType ret = func(LuaValue<Parameters>::pop(state)...);

                    return LuaValue<ReturnType>::push(state, ret);
                }

                template <
                    typename ReturnType,
                    typename = typename std::enable_if_t<!std::is_void_v<ReturnType>>>
                i32 handle_function_invocation(LuaHandle state, ReturnType (*func)()) {
                    // No parameters but a return value, just call then
                    // report number of items pushed onto Lua stack.

                    ReturnType ret = func();

                    return LuaValue<ReturnType>::push(state, ret);
                }

                template <
                    typename ReturnType,
                    typename... Parameters,
                    typename = typename std::enable_if_t<std::is_void_v<ReturnType>>>
                i32 handle_function_invocation(
                    LuaHandle state, void (*func)(Parameters...)
                ) {
                    // Parameters but no return value, pop parameters off
                    // Lua stack, call with these, then report zero items
                    // added to Lua stack.

                    func(LuaValue<Parameters>::pop(state)...);

                    return 0;
                }

                template <
                    typename ReturnType,
                    typename = typename std::enable_if_t<std::is_void_v<ReturnType>>>
                i32 handle_function_invocation(LuaHandle, void (*func)()) {
                    // No parameters, no return value, just call and
                    // report zero items added to Lua stack.

                    func();
                    return 0;
                }

                template <
                    typename Closure,
                    typename ReturnType,
                    typename... Parameters,
                    typename = typename std::enable_if_t<!std::is_void_v<ReturnType>>>
                i32 handle_closure_invocation(
                    LuaHandle state, Closure* closure, ReturnType (*func)(Parameters...)
                ) {
                    // Parameters and a return value, pop parameters off
                    // Lua stack, call with these, then report number of
                    // items pushed onto Lua stack.

                    ReturnType ret
                        = (closure->*func)(LuaValue<Parameters>::pop(state)...);

                    return LuaValue<ReturnType>::push(state, ret);
                }

                template <
                    typename Closure,
                    typename ReturnType,
                    typename = typename std::enable_if_t<!std::is_void_v<ReturnType>>>
                i32 handle_closure_invocation(
                    LuaHandle state, Closure* closure, ReturnType (*func)()
                ) {
                    // No parameters but a return value, just call then
                    // report number of items pushed onto Lua stack.

                    ReturnType ret = (closure->*func)();

                    return LuaValue<ReturnType>::push(state, ret);
                }

                template <
                    typename Closure,
                    typename ReturnType,
                    typename... Parameters,
                    typename = typename std::enable_if_t<std::is_void_v<ReturnType>>>
                i32 handle_closure_invocation(
                    LuaHandle state, Closure* closure, void (*func)(Parameters...)
                ) {
                    // Parameters but no return value, pop parameters off
                    // Lua stack, call with these, then report zero items
                    // added to Lua stack.

                    (closure->*func)(LuaValue<Parameters>::pop(state)...);

                    return 0;
                }

                template <
                    typename Closure,
                    typename ReturnType,
                    typename = typename std::enable_if_t<std::is_void_v<ReturnType>>>
                i32
                handle_closure_invocation(LuaHandle, Closure* closure, void (*func)()) {
                    // No parameters, no return value, just call and
                    // report zero items added to Lua stack.

                    (closure->*func)();
                    return 0;
                }

                template <
                    typename ReturnType,
                    typename... Parameters,
                    std::enable_if_t<!std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_delegate_invocation(
                    LuaHandle                                             state,
                    Delegate<YieldableResult<ReturnType>(Parameters...)>* delegate
                ) {
                    // Parameters and a return value, pop parameters off
                    // Lua stack, call with these, then report number of
                    // items pushed onto Lua stack.

                    bool       do_yield;
                    ReturnType ret;
                    std::tie(do_yield, ret)
                        = (*delegate)(LuaValue<Parameters>::pop(state)...);

                    return { do_yield, LuaValue<ReturnType>::push(state, ret) };
                }

                template <
                    typename ReturnType,
                    std::enable_if_t<!std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_delegate_invocation(
                    LuaHandle state, Delegate<YieldableResult<ReturnType>>* delegate
                ) {
                    // No parameters but a return value, just call then
                    // report number of items pushed onto Lua stack.

                    bool       do_yield;
                    ReturnType ret;
                    std::tie(do_yield, ret) = (*delegate)();

                    return { do_yield, LuaValue<ReturnType>::push(state, ret) };
                }

                template <
                    typename ReturnType,
                    typename... Parameters,
                    std::enable_if_t<std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_delegate_invocation(
                    LuaHandle state, Delegate<bool(Parameters...)>* delegate
                ) {
                    // Parameters but no return value, pop parameters off
                    // Lua stack, call with these, then report zero items
                    // added to Lua stack.

                    return { (*delegate)(LuaValue<Parameters>::pop(state)...), 0 };
                }

                template <
                    typename ReturnType,
                    std::enable_if_t<std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_delegate_invocation(
                    LuaHandle, Delegate<bool()>* delegate
                ) {
                    // No parameters, no return value, just call and
                    // report zero items added to Lua stack.

                    return { (*delegate)(), 0 };
                }

                template <
                    typename ReturnType,
                    typename... Parameters,
                    std::enable_if_t<!std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_function_invocation(
                    LuaHandle state, YieldableResult<ReturnType> (*func)(Parameters...)
                ) {
                    // Parameters and a return value, pop parameters off
                    // Lua stack, call with these, then report number of
                    // items pushed onto Lua stack.

                    bool       do_yield;
                    ReturnType ret;
                    std::tie(do_yield, ret) = func(LuaValue<Parameters>::pop(state)...);

                    return { do_yield, LuaValue<ReturnType>::push(state, ret) };
                }

                template <
                    typename ReturnType,
                    std::enable_if_t<!std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_function_invocation(
                    LuaHandle state, YieldableResult<ReturnType> (*func)()
                ) {
                    // No parameters but a return value, just call then
                    // report number of items pushed onto Lua stack.

                    bool       do_yield;
                    ReturnType ret;
                    std::tie(do_yield, ret) = func();

                    return { do_yield, LuaValue<ReturnType>::push(state, ret) };
                }

                template <
                    typename ReturnType,
                    typename... Parameters,
                    std::enable_if_t<std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_function_invocation(
                    LuaHandle state, bool (*func)(Parameters...)
                ) {
                    // Parameters but no return value, pop parameters off
                    // Lua stack, call with these, then report zero items
                    // added to Lua stack.

                    return { func(LuaValue<Parameters>::pop(state)...), 0 };
                }

                template <
                    typename ReturnType,
                    std::enable_if_t<std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32>
                handle_yieldable_function_invocation(LuaHandle, bool (*func)()) {
                    // No parameters, no return value, just call and
                    // report zero items added to Lua stack.

                    return { func(), 0 };
                }

                template <
                    typename Closure,
                    typename ReturnType,
                    typename... Parameters,
                    std::enable_if_t<!std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_closure_invocation(
                    LuaHandle state,
                    Closure*  closure,
                    YieldableResult<ReturnType> (*func)(Parameters...)
                ) {
                    // Parameters and a return value, pop parameters off
                    // Lua stack, call with these, then report number of
                    // items pushed onto Lua stack.

                    bool       do_yield;
                    ReturnType ret;
                    std::tie(do_yield, ret)
                        = (closure->*func)(LuaValue<Parameters>::pop(state)...);

                    return { do_yield, LuaValue<ReturnType>::push(state, ret) };
                }

                template <
                    typename Closure,
                    typename ReturnType,
                    std::enable_if_t<!std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_closure_invocation(
                    LuaHandle state,
                    Closure*  closure,
                    YieldableResult<ReturnType> (*func)()
                ) {
                    // No parameters but a return value, just call then
                    // report number of items pushed onto Lua stack.

                    bool       do_yield;
                    ReturnType ret;
                    std::tie(do_yield, ret) = (closure->*func)();

                    return { do_yield, LuaValue<ReturnType>::push(state, ret) };
                }

                template <
                    typename Closure,
                    typename ReturnType,
                    typename... Parameters,
                    std::enable_if_t<std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_closure_invocation(
                    LuaHandle state, Closure* closure, bool (*func)(Parameters...)
                ) {
                    // Parameters but no return value, pop parameters off
                    // Lua stack, call with these, then report zero items
                    // added to Lua stack.

                    return { (closure->*func)(LuaValue<Parameters>::pop(state)...), 0 };
                }

                template <
                    typename Closure,
                    typename ReturnType,
                    std::enable_if_t<std::is_void_v<ReturnType>, int> = 0>
                std::tuple<bool, i32> handle_yieldable_closure_invocation(
                    LuaHandle, Closure* closure, bool (*func)()
                ) {
                    // No parameters, no return value, just call and
                    // report zero items added to Lua stack.

                    return { (closure->*func)(), 0 };
                }
            }  // namespace impl
        }      // namespace lua
    }          // namespace script
}  // namespace hemlock

template <typename ReturnType, typename... Parameters>
i32 hscript::lua::invoke_delegate(LuaHandle state) {
    using DelegateType = Delegate<ReturnType(Parameters...)>;

    DelegateType* delegate
        = reinterpret_cast<DelegateType*>(lua_touserdata(state, lua_upvalueindex(1)));

    return impl::handle_delegate_invocation<ReturnType, Parameters...>(state, delegate);
}

template <typename ReturnType, typename... Parameters>
i32 hscript::lua::invoke_function(LuaHandle state) {
    using FuncType = ReturnType (*)(Parameters...);

    FuncType* func
        = reinterpret_cast<FuncType*>(lua_touserdata(state, lua_upvalueindex(1)));

    return impl::handle_function_invocation(state, func);
}

template <typename Closure, typename ReturnType, typename... Parameters>
i32 hscript::lua::invoke_closure(LuaHandle state) {
    using FuncType = ReturnType (*)(Parameters...);

    Closure* closure
        = reinterpret_cast<Closure*>(lua_touserdata(state, lua_upvalueindex(1)));
    FuncType* func
        = reinterpret_cast<FuncType*>(lua_touserdata(state, lua_upvalueindex(2)));

    return impl::handle_closure_invocation(state, closure, func);
}

template <typename ReturnType, typename... Parameters>
i32 hscript::lua::invoke_yieldable_delegate(LuaHandle state) {
    using DelegateType = Delegate<YieldableResult<ReturnType>(Parameters...)>;

    DelegateType* delegate
        = reinterpret_cast<DelegateType*>(lua_touserdata(state, lua_upvalueindex(1)));

    bool do_yield;
    i32  ret_count;
    std::tie(do_yield, ret_count)
        = impl::handle_yieldable_delegate_invocation<ReturnType, Parameters...>(
            state, delegate
        );

    if (do_yield) {
        return lua_yield(state, ret_count);
    } else {
        return ret_count;
    }
}

template <typename ReturnType, typename... Parameters>
i32 hscript::lua::invoke_yieldable_function(LuaHandle state) {
    using FuncType = YieldableResult<ReturnType> (*)(Parameters...);

    FuncType* func
        = reinterpret_cast<FuncType*>(lua_touserdata(state, lua_upvalueindex(1)));

    bool do_yield;
    i32  ret_count;
    std::tie(do_yield, ret_count)
        = impl::handle_yieldable_function_invocation(state, func);

    if (do_yield) {
        return lua_yield(state, ret_count);
    } else {
        return ret_count;
    }
}

template <typename Closure, typename ReturnType, typename... Parameters>
i32 hscript::lua::invoke_yieldable_closure(LuaHandle state) {
    using FuncType = YieldableResult<ReturnType> (*)(Parameters...);

    Closure* closure
        = reinterpret_cast<Closure*>(lua_touserdata(state, lua_upvalueindex(1)));
    FuncType* func
        = reinterpret_cast<FuncType*>(lua_touserdata(state, lua_upvalueindex(2)));

    bool do_yield;
    i32  ret_count;
    std::tie(do_yield, ret_count)
        = impl::handle_yieldable_closure_invocation(state, closure, func);

    if (do_yield) {
        return lua_yield(state, ret_count);
    } else {
        return ret_count;
    }
}

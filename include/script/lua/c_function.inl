#include "script/lua/lua_value.hpp"

namespace hemlock {
    namespace script {
        namespace lua {

            namespace impl {
                template <
                    typename ReturnType,
                    typename... Parameters,
                    typename
                    = typename std::enable_if_t<!std::is_same<void, ReturnType>::value>>
                i32 handle_delegate_invocation(
                    LuaHandle state, Delegate<ReturnType, Parameters...>* delegate
                ) {
                    // Parameters and a return value, pop parameters off
                    // Lua stack, call with these, then report number of
                    // items pushed onto Lua stack.

                    ReturnType ret = (*delegate)(LuaValue<Parameters>::pop(state)...);

                    return LuaValue<ReturnType>::push(state, ret);
                }

                template <
                    typename ReturnType,
                    typename
                    = typename std::enable_if_t<!std::is_same<void, ReturnType>::value>>
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
                    typename
                    = typename std::enable_if_t<!std::is_same<void, ReturnType>::value>>
                i32 handle_delegate_invocation(
                    LuaHandle state, Delegate<void, Parameters...>* delegate
                ) {
                    // Parameters but no return value, pop parameters off
                    // Lua stack, call with these, then report zero items
                    // added to Lua stack.

                    (*delegate)(LuaValue<Parameters>::pop(state)...);

                    return 0;
                }

                template <
                    typename ReturnType,
                    typename
                    = typename std::enable_if_t<!std::is_same<void, ReturnType>::value>>
                i32 handle_delegate_invocation(LuaHandle, Delegate<void()>* delegate) {
                    // No parameters, no return value, just call and
                    // report zero items added to Lua stack.

                    (*delegate)();
                    return 0;
                }

                template <
                    typename ReturnType,
                    typename... Parameters,
                    typename
                    = typename std::enable_if_t<!std::is_same<void, ReturnType>::value>>
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
                    typename
                    = typename std::enable_if_t<!std::is_same<void, ReturnType>::value>>
                i32 handle_function_invocation(LuaHandle state, ReturnType (*func)()) {
                    // No parameters but a return value, just call then
                    // report number of items pushed onto Lua stack.

                    ReturnType ret = func();

                    return LuaValue<ReturnType>::push(state, ret);
                }

                template <
                    typename ReturnType,
                    typename... Parameters,
                    typename
                    = typename std::enable_if_t<std::is_same<void, ReturnType>::value>>
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
                    typename
                    = typename std::enable_if_t<std::is_same<void, ReturnType>::value>>
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
                    typename
                    = typename std::enable_if_t<!std::is_same<void, ReturnType>::value>>
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
                    typename
                    = typename std::enable_if_t<!std::is_same<void, ReturnType>::value>>
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
                    typename
                    = typename std::enable_if_t<std::is_same<void, ReturnType>::value>>
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
                    typename
                    = typename std::enable_if_t<std::is_same<void, ReturnType>::value>>
                i32
                handle_closure_invocation(LuaHandle, Closure* closure, void (*func)()) {
                    // No parameters, no return value, just call and
                    // report zero items added to Lua stack.

                    (closure->*func)();
                    return 0;
                }
            }  // namespace impl
        }      // namespace lua
    }          // namespace script
}  // namespace hemlock

template <typename ReturnType, typename... Parameters>
i32 hscript::lua::invoke_delegate(LuaHandle state) {
    using DelegateType = Delegate<ReturnType, Parameters...>;

    DelegateType* delegate
        = reinterpret_cast<DelegateType*>(lua_touserdata(state, lua_upvalueindex(1)));

    return handle_delegate_invocation(state, delegate);
}

template <typename ReturnType, typename... Parameters>
i32 hscript::lua::invoke_function(LuaHandle state) {
    using FuncType = ReturnType (*)(Parameters...);

    FuncType* func
        = reinterpret_cast<FuncType*>(lua_touserdata(state, lua_upvalueindex(1)));

    return handle_function_invocation(state, func);
}

template <typename Closure, typename ReturnType, typename... Parameters>
i32 hscript::lua::invoke_closure(LuaHandle state) {
    using FuncType = ReturnType (*)(Parameters...);

    Closure* closure
        = reinterpret_cast<Closure*>(lua_touserdata(state, lua_upvalueindex(1)));
    FuncType* func
        = reinterpret_cast<FuncType*>(lua_touserdata(state, lua_upvalueindex(2)));

    return handle_closure_invocation(state, closure, func);
}

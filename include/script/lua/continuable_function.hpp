#ifndef __hemlock_script_lua_continuable_function_hpp
#define __hemlock_script_lua_continuable_function_hpp

#include "script/continuable_function.hpp"
#include "script/lua/lua_value.hpp"
#include "script/lua/state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            template <typename NewCallSignature, typename ContinuationSignature>
            class LuaContinuableFunction {
                // Empty.
            };

            template <
                typename ReturnType,
                typename... NewCallParameters,
                typename... ContinuationParameters>
            class LuaContinuableFunction<
                std::tuple<ReturnType, NewCallParameters...>,
                std::tuple<ReturnType, ContinuationParameters...>> :
                public ContinuableFunction<
                    LuaContinuableFunction<
                        std::tuple<ReturnType, NewCallParameters...>,
                        std::tuple<ReturnType, ContinuationParameters...>>,
                    std::tuple<ReturnType, NewCallParameters...>,
                    std::tuple<ReturnType, ContinuationParameters...>> {
                using _Base = ContinuableFunction<
                    LuaContinuableFunction<
                        std::tuple<ReturnType, NewCallParameters...>,
                        std::tuple<ReturnType, ContinuationParameters...>>,
                    std::tuple<ReturnType, NewCallParameters...>,
                    std::tuple<ReturnType, ContinuationParameters...>>;
            public:
                void init(LuaFunctionState function) { m_function = function; }

                void attach_to_thread(LuaThreadState thread);
                void detach_from_thread();
            protected:
                std::enable_if_t<
                    sizeof...(NewCallParameters) != 1
                        || !(... && std::is_same_v<NewCallParameters, void>),
                    ContinuationResult<ReturnType>>
                invoke_new_call(NewCallParameters&&... parameters) {
                    return do_invocation<NewCallParameters...>(
                        std::forward<NewCallParameters>(parameters)...
                    );
                }

                std::enable_if_t<
                    sizeof...(NewCallParameters) == 1
                        && (... && std::is_same_v<NewCallParameters, void>),
                    ContinuationResult<ReturnType>>
                invoke_new_call() {
                    return do_invocation<NewCallParameters...>();
                }

                std::enable_if_t<
                    sizeof...(ContinuationParameters) != 1
                        || !(... && std::is_same_v<ContinuationParameters, void>),
                    ContinuationResult<ReturnType>>
                invoke_continuation(ContinuationParameters&&... parameters) {
                    return do_invocation<ContinuationParameters...>(
                        std::forward<ContinuationParameters>(parameters)...
                    );
                }

                std::enable_if_t<
                    sizeof...(ContinuationParameters) == 1
                        && (... && std::is_same_v<ContinuationParameters, void>),
                    ContinuationResult<ReturnType>>
                invoke_continuation() {
                    return do_invocation<ContinuationParameters...>();
                }

                template <typename... Parameters>
                std::enable_if_t<
                    sizeof...(Parameters) != 1
                        || !(... && std::is_same_v<Parameters, void>),
                    ContinuationResult<ReturnType>>
                do_invocation(Parameters&&... parameters) {
                    i32 prior_index = lua_gettop(m_thread.thread);

                    (LuaValue<NewCallParameters>::push(m_thread.thread, parameters),
                     ...);

                    // Get number of items pushed.
                    i32 number_items
                        = static_cast<i32>(total_value_count<NewCallParameters...>());

                    auto result = lua_resume(m_thread.thread, number_items);

                    // Call the function with the provided parameters.
                    if (result == 0) {
                        _Base::m_is_yielded = false;
                    } else if (result == LUA_YIELD) {
                        _Base::m_is_yielded = true;
                    } else {
                        debug_printf(
                            "Error calling Lua function: %s.\n",
                            LuaValue<const char*>::pop(state)
                        );

                        return result;
                    }

                    if constexpr (!std::is_same<ReturnType, void>()) {
                        // Try to pop return value from Lua stack, report
                        // failure if we can't.
                        ContinuationResult<ReturnType> ret = { result, {} };
                        if (!LuaValue<ReturnType>::try_pop(m_thread.thread, ret[1])) {
                            debug_printf(
                                "Could not obtain return value from Lua function."
                            );

                            ret[0] = HEMLOCK_SCRIPT_RETURN_TYPE_ERR;
                        }

                        // Restore stack to prior state and return
                        lua_pop(
                            m_thread.thread, lua_gettop(m_thread.thread) - prior_index
                        );
                        return ret;
                    } else {
                        // Restore stack to prior state and return
                        lua_pop(
                            m_thread.thread, lua_gettop(m_thread.thread) - prior_index
                        );
                        return result;
                    }
                }

                template <typename... Parameters>
                std::enable_if_t<
                    sizeof...(Parameters) == 1
                        && (... && std::is_same_v<Parameters, void>),
                    ContinuationResult<ReturnType>>
                do_invocation() {
                    i32 prior_index = lua_gettop(m_thread.thread);

                    auto result = lua_resume(m_thread.thread, 0);

                    // Call the function with the provided parameters.
                    if (result == 0) {
                        _Base::m_is_yielded = false;
                    } else if (result == LUA_YIELD) {
                        _Base::m_is_yielded = true;
                    } else {
                        debug_printf(
                            "Error calling Lua function: %s.\n",
                            LuaValue<const char*>::pop(state)
                        );

                        return result;
                    }

                    if constexpr (!std::is_same<ReturnType, void>()) {
                        // Try to pop return value from Lua stack, report
                        // failure if we can't.
                        ContinuationResult<ReturnType> ret = { result, {} };
                        if (!LuaValue<ReturnType>::try_pop(m_thread.thread, ret[1])) {
                            debug_printf(
                                "Could not obtain return value from Lua function."
                            );

                            ret[0] = HEMLOCK_SCRIPT_RETURN_TYPE_ERR;
                        }

                        // Restore stack to prior state and return
                        lua_pop(
                            m_thread.thread, lua_gettop(m_thread.thread) - prior_index
                        );
                        return ret;
                    } else {
                        // Restore stack to prior state and return
                        lua_pop(
                            m_thread.thread, lua_gettop(m_thread.thread) - prior_index
                        );
                        return result;
                    }
                }

                LuaThreadState   m_thread;
                LuaFunctionState m_function;
            };
        }  // namespace lua
    }      // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#include "continuable_function.inl"

#endif  // __hemlock_script_lua_continuable_function_hpp

#ifndef __hemlock_script_lua_continuable_function_hpp
#define __hemlock_script_lua_continuable_function_hpp

#include "script/continuable_function.hpp"
#include "script/lua/lua_value.hpp"
#include "script/lua/state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            class LuaContinuableFunction :
                public ContinuableFunction<LuaContinuableFunction> {
                friend class ContinuableFunction<LuaContinuableFunction>;

                using _Base = ContinuableFunction<LuaContinuableFunction>;
            public:
                LuaContinuableFunction() : _Base(), m_thread{}, m_function{} {
                    // Empty.
                }

                void init(LuaFunctionState function) { m_function = function; }

                void attach_to_thread(LuaThreadState thread);
                void detach_from_thread();

                HEMLOCK_NOINLINE i32 force_yield() {
                    return lua_yield(m_thread.thread, 0);
                }
            protected:
                template <typename ReturnType, typename... Parameters>
                std::enable_if_t<
                    sizeof...(Parameters) != 1
                        || !(... && std::is_same_v<Parameters, void>),
                    ContinuationResult<ReturnType>>
                invoke_new_call(Parameters&&... parameters) {
                    return do_invocation(std::forward<Parameters>(parameters)...);
                }

                template <typename ReturnType>
                ContinuationResult<ReturnType> invoke_new_call() {
                    return do_invocation<ReturnType>();
                }

                template <typename ReturnType, typename... Parameters>
                std::enable_if_t<
                    sizeof...(Parameters) != 1
                        || !(... && std::is_same_v<Parameters, void>),
                    ContinuationResult<ReturnType>>
                invoke_continuation(Parameters&&... parameters) {
                    return do_invocation(std::forward<Parameters>(parameters)...);
                }

                template <typename ReturnType>
                ContinuationResult<ReturnType> invoke_continuation() {
                    return do_invocation<ReturnType>();
                }

                template <typename ReturnType, typename... Parameters>
                std::enable_if_t<
                    sizeof...(Parameters) != 1
                        || !(... && std::is_same_v<Parameters, void>),
                    ContinuationResult<ReturnType>>
                do_invocation(Parameters&&... parameters) {
                    i32 prior_index = lua_gettop(m_thread.thread);

                    (LuaValue<Parameters>::push(m_thread.thread, parameters), ...);

                    // Get number of items pushed.
                    i32 number_items
                        = static_cast<i32>(total_value_count<Parameters...>());

                    auto result = lua_resume(m_thread.thread, number_items);

                    // Call the function with the provided parameters.
                    if (result == 0) {
                        _Base::m_is_yielded = false;
                    } else if (result == LUA_YIELD) {
                        _Base::m_is_yielded = true;
                    } else {
                        debug_printf(
                            "Error calling Lua function: %s.\n",
                            LuaValue<const char*>::pop(m_thread.thread)
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

                template <typename ReturnType>
                ContinuationResult<ReturnType> do_invocation() {
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
                            LuaValue<const char*>::pop(m_thread.thread)
                        );

                        return { result, {} };
                    }

                    if constexpr (!std::is_same<ReturnType, void>()) {
                        // Try to pop return value from Lua stack, report
                        // failure if we can't.
                        ReturnType func_result = {};
                        if (!LuaValue<ReturnType>::try_pop(
                                m_thread.thread, func_result
                            ))
                        {
                            debug_printf(
                                "Could not obtain return value from Lua function."
                            );

                            result = HEMLOCK_SCRIPT_RETURN_TYPE_ERR;
                        }

                        ContinuationResult<ReturnType> ret = { result, func_result };

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

#endif  // __hemlock_script_lua_continuable_function_hpp

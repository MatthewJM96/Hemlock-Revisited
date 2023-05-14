#ifndef __hemlock_script_lua_continuable_function_hpp
#define __hemlock_script_lua_continuable_function_hpp

#include "script/continuable_function.hpp"
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
                ContinuationResult<ReturnType>
                invoke_new_call(NewCallParameters&&... parameters);
                ContinuationResult<ReturnType>
                invoke_continuation(ContinuationParameters&&... parameters);

                template <typename... Parameters>
                ContinuationResult<ReturnType> do_invocation(Parameters&&... parameters
                );

                LuaThreadState   m_thread;
                LuaFunctionState m_function;
            };
        }  // namespace lua
    }      // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#include "continuable_function.inl"

#endif  // __hemlock_script_lua_continuable_function_hpp

#ifndef __hemlock_script_lua_continuable_function_hpp
#define __hemlock_script_lua_continuable_function_hpp

#include "script/continuable_function.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            template <typename NewCallSignature, typename ContinuationSignature>
            class LuaContinuableFunction {
                // Empty.
            };

            template <
                typename NewCallReturnType,
                typename ContinuationReturnType,
                typename... NewCallParameters,
                typename... ContinuationParameters>
            class LuaContinuableFunction<
                std::tuple<NewCallReturnType, NewCallParameters...>,
                std::tuple<ContinuationReturnType, ContinuationParameters...>> :
                public ContinuableFunction<
                    LuaContinuableFunction<
                        std::tuple<NewCallReturnType, NewCallParameters...>,
                        std::tuple<ContinuationReturnType, ContinuationParameters...>>,
                    std::tuple<NewCallReturnType, NewCallParameters...>,
                    std::tuple<ContinuationReturnType, ContinuationParameters...>> {
            protected:
                NewCallReturnType invoke_new_call(NewCallParameters&&... parameters) {
                    // Implement.
                }

                ContinuationReturnType
                invoke_continuation(ContinuationParameters&&... parameters) {
                    // Implement.
                }

                bool m_is_yielded;
            };
        }  // namespace lua
    }      // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_lua_continuable_function_hpp

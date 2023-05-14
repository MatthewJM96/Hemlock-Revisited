#ifndef __hemlock_script_continuable_function_hpp
#define __hemlock_script_continuable_function_hpp

namespace hemlock {
    namespace script {
        template <
            typename ContinuableFunctionImpl,
            typename NewCallSignature,
            typename ContinuationSignature,
            typename = void>
        class ContinuableFunction {
            // Empty.
        };

        template <
            typename ContinuableFunctionImpl,
            typename NewCallReturnType,
            typename ContinuationReturnType,
            typename... NewCallParameters,
            typename... ContinuationParameters>
        class ContinuableFunction<
            ContinuableFunctionImpl,
            std::tuple<NewCallReturnType, NewCallParameters...>,
            std::tuple<ContinuationReturnType, ContinuationParameters...>,
            std::enable_if_t<
                std::is_same_v<NewCallReturnType, ContinuationReturnType>
                && (...
                    && std::is_same_v<NewCallParameters, ContinuationParameters>)>> {
        public:
            ContinuableFunction() : m_is_yielded(false) { /* Empty. */
            }

            NewCallReturnType operator()(NewCallParameters&&... parameters) {
                if (m_is_yielded) {
                    return invoke_new_call(std::forward<NewCallParameters>(parameters
                    )...);
                } else {
                    return invoke_continuation(
                        std::forward<NewCallParameters>(parameters)...
                    );
                }
            }
        protected:
            NewCallReturnType invoke_new_call(NewCallParameters&&... parameters) {
                reinterpret_cast<ContinuableFunctionImpl*>(this)->invoke_new_call(
                    std::forward<NewCallParameters>(parameters)...
                );
            }

            ContinuationReturnType
            invoke_continuation(ContinuationParameters&&... parameters) {
                reinterpret_cast<ContinuableFunctionImpl*>(this)->invoke_continuation(
                    std::forward<NewCallParameters>(parameters)...
                );
            }

            bool m_is_yielded;
        };

        template <
            typename ContinuableFunctionImpl,
            typename NewCallReturnType,
            typename ContinuationReturnType,
            typename... NewCallParameters,
            typename... ContinuationParameters>
        class ContinuableFunction<
            ContinuableFunctionImpl,
            std::tuple<NewCallReturnType, NewCallParameters...>,
            std::tuple<ContinuationReturnType, ContinuationParameters...>,
            std::enable_if_t<
                !std::is_same_v<NewCallReturnType, ContinuationReturnType>
                || !(... && std::is_same_v<NewCallParameters, ContinuationParameters>)>> {
        public:
            ContinuableFunction() : m_is_yielded(false) { /* Empty. */
            }

            NewCallReturnType operator()(NewCallParameters&&... parameters) {
#if DEBUG
                assert(!m_is_yielded);
#endif
                return invoke_new_call(std::forward<NewCallParameters>(parameters)...);
            }

            ContinuationReturnType operator()(ContinuationParameters&&... parameters) {
#if DEBUG
                assert(m_is_yielded);
#endif
                return invoke_continuation(
                    std::forward<ContinuationParameters>(parameters)...
                );
            }
        protected:
            NewCallReturnType invoke_new_call(NewCallParameters&&... parameters) {
                reinterpret_cast<ContinuableFunctionImpl*>(this)->invoke_new_call(
                    std::forward<NewCallParameters>(parameters)...
                );
            }

            ContinuationReturnType
            invoke_continuation(ContinuationParameters&&... parameters) {
                reinterpret_cast<ContinuableFunctionImpl*>(this)->invoke_continuation(
                    std::forward<NewCallParameters>(parameters)...
                );
            }

            bool m_is_yielded;
        };
    }  // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_continuable_function_hpp

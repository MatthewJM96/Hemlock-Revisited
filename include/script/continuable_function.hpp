#ifndef __hemlock_script_continuable_function_hpp
#define __hemlock_script_continuable_function_hpp

namespace hemlock {
    namespace script {
        template <typename ReturnType>
        using ContinuationResult = std::
            conditional_t<std::is_void_v<ReturnType>, i32, std::tuple<i32, ReturnType>>;

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
            typename ReturnType,
            typename... NewCallParameters,
            typename... ContinuationParameters>
        class ContinuableFunction<
            ContinuableFunctionImpl,
            std::tuple<ReturnType, NewCallParameters...>,
            std::tuple<ReturnType, ContinuationParameters...>,
            std::enable_if_t<
                (... && std::is_same_v<NewCallParameters, ContinuationParameters>)>> {
        public:
            ContinuableFunction() : m_is_yielded(false) { /* Empty. */
            }

            std::enable_if_t<
                sizeof...(NewCallParameters) != 1
                    || !(... && std::is_same_v<NewCallParameters, void>),
                ContinuationResult<ReturnType>>
            operator()(NewCallParameters&&... parameters) {
                if (m_is_yielded) {
                    return invoke_new_call(std::forward<NewCallParameters>(parameters
                    )...);
                } else {
                    return invoke_continuation(
                        std::forward<NewCallParameters>(parameters)...
                    );
                }
            }

            std::enable_if_t<
                sizeof...(NewCallParameters) == 1
                    && (... && std::is_same_v<NewCallParameters, void>),
                ContinuationResult<ReturnType>>
            operator()() {
                if (m_is_yielded) {
                    return invoke_new_call();
                } else {
                    return invoke_continuation();
                }
            }

            bool is_yielded() { return m_is_yielded; }
        protected:
            std::enable_if_t<
                sizeof...(NewCallParameters) != 1
                    || !(... && std::is_same_v<NewCallParameters, void>),
                ContinuationResult<ReturnType>>
            invoke_new_call(NewCallParameters&&... parameters) {
                reinterpret_cast<ContinuableFunctionImpl*>(this)->invoke_new_call(
                    std::forward<NewCallParameters>(parameters)...
                );
            }

            std::enable_if_t<
                sizeof...(NewCallParameters) == 1
                    && (... && std::is_same_v<NewCallParameters, void>),
                ContinuationResult<ReturnType>>
            invoke_new_call() {
                reinterpret_cast<ContinuableFunctionImpl*>(this)->invoke_new_call();
            }

            std::enable_if_t<
                sizeof...(NewCallParameters) != 1
                    || !(... && std::is_same_v<NewCallParameters, void>),
                ContinuationResult<ReturnType>>
            invoke_continuation(NewCallParameters&&... parameters) {
                reinterpret_cast<ContinuableFunctionImpl*>(this)->invoke_continuation(
                    std::forward<NewCallParameters>(parameters)...
                );
            }

            std::enable_if_t<
                sizeof...(NewCallParameters) == 1
                    && (... && std::is_same_v<NewCallParameters, void>),
                ContinuationResult<ReturnType>>
            invoke_continuation() {
                reinterpret_cast<ContinuableFunctionImpl*>(this)->invoke_continuation();
            }

            bool m_is_yielded;
        };

        template <
            typename ContinuableFunctionImpl,
            typename ReturnType,
            typename... NewCallParameters,
            typename... ContinuationParameters>
        class ContinuableFunction<
            ContinuableFunctionImpl,
            std::tuple<ReturnType, NewCallParameters...>,
            std::tuple<ReturnType, ContinuationParameters...>,
            std::enable_if_t<
                !(... && std::is_same_v<NewCallParameters, ContinuationParameters>)>> {
        public:
            ContinuableFunction() : m_is_yielded(false) { /* Empty. */
            }

            ContinuationResult<ReturnType> operator()(NewCallParameters&&... parameters
            ) {
#if DEBUG
                assert(!m_is_yielded);
#endif
                return invoke_new_call(std::forward<NewCallParameters>(parameters)...);
            }

            ContinuationResult<ReturnType>
            operator()(ContinuationParameters... parameters) {
#if DEBUG
                assert(m_is_yielded);
#endif
                return invoke_continuation(
                    std::forward<ContinuationParameters>(parameters)...
                );
            }

            bool is_yielded() { return m_is_yielded; }
        protected:
            ContinuationResult<ReturnType>
            invoke_new_call(NewCallParameters&&... parameters) {
                reinterpret_cast<ContinuableFunctionImpl*>(this)->invoke_new_call(
                    std::forward<NewCallParameters>(parameters)...
                );
            }

            ContinuationResult<ReturnType>
            invoke_continuation(ContinuationParameters&&... parameters) {
                reinterpret_cast<ContinuableFunctionImpl*>(this)->invoke_continuation(
                    std::forward<ContinuationParameters>(parameters)...
                );
            }

            bool m_is_yielded;
        };
    }  // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_continuable_function_hpp

#ifndef __hemlock_script_continuable_function_hpp
#define __hemlock_script_continuable_function_hpp

namespace hemlock {
    namespace script {
        template <typename ReturnType>
        using ContinuationResult = std::
            conditional_t<std::is_void_v<ReturnType>, i32, std::tuple<i32, ReturnType>>;

        template <typename ReturnType>
        using YieldableResult = std::conditional_t<
            std::is_void_v<ReturnType>,
            bool,
            std::tuple<bool, ReturnType>>;

        template <typename ContinuableFunctionImpl>
        class ContinuableFunction {
        public:
            ContinuableFunction() : m_is_yielded(false) { /* Empty. */
            }

            template <typename ReturnType, typename... Parameters>
            std::enable_if_t<
                sizeof...(Parameters) != 1
                    || !(... && std::is_same_v<Parameters, void>),
                ContinuationResult<ReturnType>>
            operator()(Parameters&&... parameters) {
                return invoke(std::forward<Parameters>(parameters)...);
            }

            template <typename ReturnType = void>
            ContinuationResult<ReturnType> operator()() {
                return invoke();
            }

            template <typename ReturnType, typename... Parameters>
            std::enable_if_t<
                sizeof...(Parameters) != 1
                    || !(... && std::is_same_v<Parameters, void>),
                ContinuationResult<ReturnType>>
            invoke(Parameters&&... parameters) {
                if (m_is_yielded) {
                    return invoke_new_call(std::forward<Parameters>(parameters)...);
                } else {
                    return invoke_continuation(std::forward<Parameters>(parameters)...);
                }
            }

            template <typename ReturnType = void>
            ContinuationResult<ReturnType> invoke() {
                if (m_is_yielded) {
                    return invoke_new_call<ReturnType>();
                } else {
                    return invoke_continuation<ReturnType>();
                }
            }

            i32 force_yield() {
                return reinterpret_cast<ContinuableFunctionImpl*>(this)->force_yield();
            }

            bool is_yielded() { return m_is_yielded; }
        protected:
            template <typename ReturnType, typename... Parameters>
            std::enable_if_t<
                sizeof...(Parameters) != 1
                    || !(... && std::is_same_v<Parameters, void>),
                ContinuationResult<ReturnType>>
            invoke_new_call(Parameters&&... parameters) {
                return reinterpret_cast<ContinuableFunctionImpl*>(this)
                    ->invoke_new_call(std::forward<Parameters>(parameters)...);
            }

            template <typename ReturnType>
            ContinuationResult<ReturnType> invoke_new_call() {
                return reinterpret_cast<ContinuableFunctionImpl*>(this)
                    ->template invoke_new_call<ReturnType>();
            }

            template <typename ReturnType, typename... Parameters>
            std::enable_if_t<
                sizeof...(Parameters) != 1
                    || !(... && std::is_same_v<Parameters, void>),
                ContinuationResult<ReturnType>>
            invoke_continuation(Parameters&&... parameters) {
                return reinterpret_cast<ContinuableFunctionImpl*>(this)
                    ->invoke_continuation(std::forward<Parameters>(parameters)...);
            }

            template <typename ReturnType>
            ContinuationResult<ReturnType> invoke_continuation() {
                return reinterpret_cast<ContinuableFunctionImpl*>(this)
                    ->template invoke_continuation<ReturnType>();
            }

            bool m_is_yielded;
        };
    }  // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_continuable_function_hpp

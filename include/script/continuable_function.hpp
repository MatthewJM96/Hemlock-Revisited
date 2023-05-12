#ifndef __hemlock_script_continuable_function_hpp
#define __hemlock_script_continuable_function_hpp

namespace hemlock {
    namespace script {
        template <
            typename NewCallSignature,
            typename ContinuationSignature,
            typename = void>
        class ContinuableFunction {
            // Empty.
        };

        template <
            typename NewCallReturnType,
            typename... NewCallParameters,
            typename ContinuationReturnType,
            typename... ContinuationParameters>
        class ContinuableFunction<
            std::tuple<NewCallReturnType, NewCallParameters...>,
            std::tuple<ContinuationReturnType, ContinuationParameters...>,
            std::enable_if_t<
                std::is_same_v<NewCallReturnType, ContinuationReturnType>
                && (...
                    && std::is_same_v<NewCallParameters, ContinuationParameters>)>> {
        public:
            ContinuableFunction() : m_is_yielded(false) { /* Empty. */
            }

            NewCallReturnType operator()(NewCallParameters... parameters) {
                if (m_is_yielded) {
                    return invoke_new_call(parameters...);
                } else {
                    return invoke_continuation(parameters...);
                }
            }
        protected:
            NewCallReturnType invoke_new_call(NewCallParameters... parameters);
            ContinuationReturnType
            invoke_continuation(ContinuationParameters... parameters);

            bool m_is_yielded;
        };

        template <
            typename NewCallReturnType,
            typename... NewCallParameters,
            typename ContinuationReturnType,
            typename... ContinuationParameters>
        class ContinuableFunction<
            std::tuple<NewCallReturnType, NewCallParameters...>,
            std::tuple<ContinuationReturnType, ContinuationParameters...>,
            std::enable_if_t<
                !std::is_same_v<NewCallReturnType, ContinuationReturnType>
                || !(... && std::is_same_v<NewCallParameters, ContinuationParameters>)>> {
        public:
            ContinuableFunction() : m_is_yielded(false) { /* Empty. */
            }

            NewCallReturnType operator()(NewCallParameters... parameters) {
#if DEBUG
                assert(!m_is_yielded);
#endif
                return invoke_new_call(parameters...);
            }

            ContinuationReturnType operator()(ContinuationParameters... parameters) {
#if DEBUG
                assert(m_is_yielded);
#endif
                return invoke_continuation(parameters...);
            }
        protected:
            NewCallReturnType invoke_new_call(NewCallParameters... parameters);
            ContinuationReturnType
            invoke_continuation(ContinuationParameters... parameters);

            bool m_is_yielded;
        };
    }  // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_continuable_function_hpp

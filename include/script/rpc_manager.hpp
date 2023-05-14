#ifndef __hemlock_script_rpc_manager_hpp
#define __hemlock_script_rpc_manager_hpp

#include "script/call_state.hpp"
#include "script/environment_base_decl.hpp"

namespace hemlock {
    namespace script {
        // TODO(Matthew): Reduce new calls as much as possible, introduce pageing and
        //                call limits. Try to remove use of std::string for calls,
        //                but not sure how to do that just yet.
        // TODO(Matthew): Single RPC manager per environment, or per environment
        // group?

        // TODO(Matthew): Automatic deqeueing of continuable calls that don't
        //                complete after some number of pumps? Likewise deletion
        //                of return values and states that don't get removed
        //                after some number of pumps?
        // TODO(Matthew): Speed? Mutex use is heavy.
        template <
            typename EnvironmentImpl,
            template <typename, typename>
            typename ContinuableFuncImpl,
            typename ThreadImpl,
            size_t BufferSize = 0>
        class RPCManager {
            using _Environment = EnvironmentBase<
                EnvironmentImpl,
                ContinuableFuncImpl,
                ThreadImpl,
                true,
                BufferSize>;
        public:
            RPCManager() :
                m_environment(nullptr),
                m_latest_call_id(0),
                m_calls_buffered(0) { /* Empty. */
            }

            ~RPCManager() { dispose(); }

            /**
             * @brief Initialises the RPC manager.
             *
             * @param env The environment this RPC manager is
             * attached to.
             * @param is_public Whether the environment is public
             * accessible or not.
             */
            void init(_Environment* env, bool is_public = true);
            /**
             * @brief Disposes the RPC manager.
             */
            void dispose();

            /**
             * @brief Appends the given call to the call buffer
             * if the buffer is not already full.
             *
             * @param call The call to append.
             * @param parameters The parameters to pass to the appended call
             * @return CallID The ID associated with the call,
             * negative if the call was not appended. (Currently
             * -1 is returned solely.)
             */
            CallID append_call(std::string&& call, CallParameters&& parameters);

            /**
             * @brief Get the state of buffered call with the
             * given ID.
             *
             * @param id The ID of the call to get the state of.
             * @param state The value populated with the state of
             * the buffered call.
             * @return i32 -1 if no call is buffered with
             * given ID, otherwise 0.
             */
            i32 call_state(CallID id, OUT CallState& state);

            /**
             * @brief Get the return values of buffered call with
             * the given ID.
             *
             * @param id The ID of the call to get the return values
             * of.
             * @param return_values The return values of the buffered
             * call.
             * @return i32 -1 if no call is buffered with given
             * ID, otherwise 0.
             */
            i32 call_return_values(CallID id, OUT CallParameters& return_values);

            /**
             * @brief Removes call with the given ID from the buffer.
             *
             * @param id The ID of the call to remove from the buffer.
             * @return i32 -1 if no call is buffered with given
             * ID, otherwise 0.
             */
            i32 remove_call(CallID id);

            /**
             * @brief Pumps calls in call buffer, handling
             * continuable functions and storing return
             * values.
             */
            void pump_calls();

            /**
             * @brief Sets whether the environment this RPC manager
             * is attached to is public or not in accessibility. A
             * public environment allows all functions within to be
             * called, while a non-public environment does not.
             *
             * @param is_public Whether the environment is public or not.
             */
            void set_is_public_env(bool is_public = true);

            /**
             * @brief Registers a function within the associated
             * environment as publicly accessible, overriding
             * the case of non-public status of the environment.
             *
             * @param function The function to register as public.
             */
            void register_public_function(std::string&& function);

            /**
             * @brief Registers a function within the associated
             * environment as a continuable function. A continuable
             * function may return as incomplete on being pumped,
             * causing it to be placed back into the command buffer
             * for calling on the next pump. Such a function is
             * useful if a call may depend on some state that
             * may be evolved to in time but is not the case in
             * one update loop.
             *
             * Note that this implies making the function publicly
             * accessible, overriding the case of non-public status
             * of the environment.
             *
             * @param function The function to register as public.
             */
            void register_continuable_function(std::string&& function);
        protected:
            _Environment* m_environment;

            CallID    m_latest_call_id;
            Calls     m_calls;
            CallsData m_call_data;

            std::mutex m_buffer_lock;

            bool                            m_is_public_env;
            std::unordered_set<std::string> m_public_functions;

            std::unordered_set<std::string> m_continuable_functions;

            typename std::conditional<
                std::greater<size_t>()(BufferSize, 0),
                size_t,
                std::monostate>::type m_calls_buffered;
        };
    }  // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#include "rpc_manager.inl"

#endif  // __hemlock_script_rpc_manager_hpp

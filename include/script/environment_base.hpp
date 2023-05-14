#ifndef __hemlock_script_environment_base_hpp
#define __hemlock_script_environment_base_hpp

#ifndef HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH
#  define HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH 50 * 1024 * 1024
#endif  // HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH

#include "script/continuable_function.hpp"
#include "script/environment_base_decl.hpp"
#include "script/rpc_manager.hpp"
#include "script/state.hpp"

// TODO(Matthew): Create a REPL client that can be used for dev console and so on.
// TODO(Matthew): Create a comms environment/plug-in that attaches to groups and
//                standalone environments to allow them to communicate with each
//                other. Acts as a command buffer taking up commands from other
//                groups/envs and running them asynchronously.

namespace hemlock {
    namespace script {
        template <typename Environment>
        class EnvironmentRegistry;

        template <
            typename EnvironmentImpl,
            template <typename, typename>
            typename ContinuableFuncImpl,
            bool   HasRPCManager /*= false*/,
            size_t CallBufferSize /*= 0*/
            >
        class EnvironmentBase {
        public:
            EnvironmentBase() { /* Empty. */
            }

            ~EnvironmentBase() { /* Empty. */
            }

            HEMLOCK_NON_COPYABLE(EnvironmentBase);

            HEMLOCK_MOVABLE(EnvironmentBase) {
                m_registry          = rhs.m_registry;
                m_io_manager        = rhs.m_io_manager;
                m_max_script_length = rhs.m_max_script_length;

                return *this;
            }

            /**
             * @brief Initialise the environment as a standalone
             * environment.
             *
             * @param io_manager The IO manager with which the
             * environment discovers scripts in load and run
             * functions.
             * @param registry Optionally the registry in which
             * this environment is registered.
             * @param max_script_length The maximum length of any
             * script that this environment will process.
             */
            virtual void init(
                hio::IOManagerBase*                   io_manager,
                EnvironmentRegistry<EnvironmentImpl>* registry = nullptr,
                ui32 max_script_length = HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH
            ) = 0;
            /**
             * @brief Initialise the environment as a child of a
             * parent environment. All children of the same parent
             * share the same global state.
             *
             * @param parent The parent environment of this one.
             * @param io_manager The IO manager with which the
             * environment discovers scripts in load and run
             * functions.
             * @param registry Optionally the registry in which
             * this environment is registered.
             * @param max_script_length The maximum length of any
             * script that this environment will process.
             */
            virtual void init(
                EnvironmentBase*                      parent,
                hio::IOManagerBase*                   io_manager,
                EnvironmentRegistry<EnvironmentImpl>* registry = nullptr,
                ui32 max_script_length = HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH
            ) = 0;
            /**
             * @brief Dispose the environment.
             */
            virtual void dispose() = 0;

            /**
             * @brief Load in a script from the provided filepath.
             *
             * @param filepath The filepath from which to load the script.
             */
            virtual bool load(const hio::fs::path& filepath) = 0;
            /**
             * @brief Load in the provided script string.
             *
             * @param script A string of script to load into the script environment.
             */
            virtual bool load(const std::string& script) = 0;

            /**
             * @brief Runs everything already loaded into the script environment.
             */
            virtual bool run() = 0;
            /**
             * @brief Loads in the script at the provided filepath, then runs it.
             *
             * Anything loaded in before this call will also be ran if no other calls
             * to run have been made since.
             *
             * @param filepath The filepath from which to load the script.
             */
            virtual bool run(const hio::fs::path& filepath) = 0;
            /**
             * @brief Loads in the provided script string, then runs it.
             *
             * Anything loaded in before this call will also be ran if no other calls
             * to run have been made since.
             *
             * @param script A string of script to load into the script environment.
             */
            virtual bool run(const std::string& script) = 0;

            /**
             * @brief Set this environment's current namespace to
             * the global namespace.
             */
            virtual void set_global_namespace() = 0;

            /**
             * @brief Set this environment's current namespace to
             * the global namespace then enter into the specified
             * namespaces from there. I.e. if namespaces pack
             * contains ("voxel", "chunk") then this function
             * sets the environment's current namespace to
             * <voxel.chunk>.
             *
             * @tparam Strings Just used to allow entering into
             * namespaces at arbitrary depth.
             * @param namespaces The namespaces to set.
             */
            template <typename... Strings>
            void set_namespaces(Strings... namespaces) {
                reinterpret_cast<EnvironmentImpl*>(this)->set_namespaces(namespaces...);
            }

            /**
             * @brief From this environment's current namespace
             * enter into the specified namespaces. I.e. if
             * namespaces pack contains ("chunk") and the
             * environment is currently in the namespace <voxel>,
             * then this function sets the environment's current
             * namespace to <voxel.chunk>.
             *
             * @tparam Strings Just used to allow entering into
             * namespaces at arbitrary depth.
             * @param namespaces The namespaces to enter.
             */
            template <typename... Strings>
            void enter_namespaces(Strings... namespaces) {
                reinterpret_cast<EnvironmentImpl*>(this)->enter_namespaces(namespaces...
                );
            }

            /**
             * @brief Add a value to the environment, exposed to the
             * scripts ran within.
             *
             * @tparam ReturnType The type of the value.
             * @param name The name to expose the value as within the
             * environment.
             * @param val The value to be added to the environment.
             */
            template <typename Type>
            void add_value(std::string_view name, Type val) {
                reinterpret_cast<EnvironmentImpl*>(this)->add_value(
                    name, std::forward<Type>(val)
                );
            }

            /**
             * @brief Add a delegate to the environment, exposed to the
             * scripts ran within.
             *
             * @tparam ReturnType The return type of the delegate.
             * @tparam Parameters The parameters accepted by the delegate.
             * @param name The name to expose the delegate as within the
             * environment.
             * @param delegate The delegate to be added to the environment.
             */
            template <typename ReturnType, typename... Parameters>
            void add_c_delegate(
                std::string_view name, Delegate<ReturnType, Parameters...>* delegate
            ) {
                reinterpret_cast<EnvironmentImpl*>(this)->add_c_delegate(
                    name, delegate
                );
            }

            /**
             * @brief Add a function to the environment, exposed to the
             * scripts ran within.
             *
             * @tparam ReturnType The return type of the function.
             * @tparam Parameters The parameters accepted by the function.
             * @param name The name to expose the function as within the
             * environment.
             * @param func The function to be added to the environment.
             */
            template <typename ReturnType, typename... Parameters>
            void
            add_c_function(std::string_view name, ReturnType (*func)(Parameters...)) {
                reinterpret_cast<EnvironmentImpl*>(this)->add_c_function(name, func);
            }

            /**
             * @brief Add a closure to the environment, exposed to the
             * scripts ran within.
             *
             * @tparam Closure The closure type.
             * @tparam ReturnType The return type of the closure's invocation.
             * @tparam Parameters The parameters accepted by the closure's invocation.
             * @param name The name to expose the closure as within the
             * environment.
             * @param closure The closure to be added to the environment.
             * @param func The closure's invocation method.
             */
            template <
                std::invocable Closure,
                typename ReturnType,
                typename... Parameters>
            void add_c_closure(
                std::string_view name,
                Closure*         closure,
                ReturnType (Closure::*func)(Parameters...)
            ) {
                reinterpret_cast<EnvironmentImpl*>(this)->add_c_closure(
                    name, closure, func
                );
            }

            /**
             * @brief Add a closure to the environment, exposed to the
             * scripts ran within. This provides a default expecation
             * of an unambiguous operator() in the Closure type.
             *
             * @tparam Closure The closure type.
             * @tparam ReturnType The return type of the closure's invocation.
             * @tparam Parameters The parameters accepted by the closure's invocation.
             * @param name The name to expose the closure as within the
             * environment.
             * @param closure The closure to be added to the environment.
             */
            template <std::invocable Closure>
            void add_c_closure(std::string_view name, Closure* closure) {
                reinterpret_cast<EnvironmentImpl*>(this)->add_c_closure(
                    name, closure, Closure::operator()
                );
            }

            /**
             * @brief Get a script function from the environment, allowing
             * calls within C++ into the script.
             *
             * @tparam ReturnType The return type of the script function.
             * @tparam Parameters The parameters accepted by the script function.
             * @param name The name of the script function to obtain.
             * @param delegate Delegate providing means to call the script function.
             * @return True if the script function was obtained, false otherwise.
             */
            template <typename ReturnType, typename... Parameters>
            bool get_script_function(
                std::string&& name,
                OUT ScriptDelegate<ReturnType, Parameters...>& delegate
            ) {
                return reinterpret_cast<EnvironmentImpl*>(this)
                    ->template get_script_function<ReturnType, Parameters...>(
                        std::move(name), delegate
                    );
            }

            /**
             * @brief Get a continuable script function from the environment, allowing
             * calls within C++ into the script where the script may yield back and be
             * continued later.
             *
             * @tparam NewCallSignature The signature of a new call to the script
             * function.
             * @tparam ContinuationCallSignature The signautre of a continuation of the
             * script function.
             * @param name The name of the script function to obtain.
             * @param continuable_function Delegate providing means to call the script
             * function.
             * @return True if the script function was obtained, false otherwise.
             */
            template <typename NewCallSignature, typename ContinuationCallSignature>
            bool get_continuable_script_function(
                std::string&& name,
                OUT ContinuableFuncImpl<NewCallSignature, ContinuationCallSignature>&
                    continuable_function
            ) {
                return reinterpret_cast<EnvironmentImpl*>(this)
                    ->template get_continuable_script_function<
                        NewCallSignature,
                        ContinuationCallSignature>(
                        std::move(name), continuable_function
                    );
            }

            typename std::conditional<
                HasRPCManager,
                RPCManager<EnvironmentImpl, ContinuableFuncImpl, CallBufferSize>,
                std::monostate>::type rpc
                = {};
        protected:
            EnvironmentRegistry<EnvironmentImpl>* m_registry;

            typename std::conditional<HasRPCManager, bool, std::monostate>::type
                m_rpc_manual_pump
                = {};

            hio::IOManagerBase* m_io_manager;
            ui32                m_max_script_length;
        };
    }  // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_environment_base_hpp

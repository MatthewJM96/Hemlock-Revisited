#ifndef __hemlock_script_lua_environment_h
#define __hemlock_script_lua_environment_h

#include "script/environment_base.hpp"
#include "script/lua/continuable_function.hpp"
#include "script/lua/lua_function.hpp"
#include "script/lua/rpc_functions.hpp"
#include "script/lua/state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            using LuaFunctions = std::unordered_map<std::string, LuaFunctionState>;

            // TODO(Matthew): Revisit handling of group environments:
            //                  - verify cache and function registration handling,
            //                  - within group parallelism with lua_xmove?

            template <bool HasRPCManager = false, size_t CallBufferSize = 0>
            class Environment :
                public EnvironmentBase<
                    Environment<HasRPCManager, CallBufferSize>,
                    LuaContinuableFunction,
                    LuaThreadState,
                    HasRPCManager,
                    CallBufferSize> {
                friend i32
                    hscript::lua::register_lua_function<HasRPCManager, CallBufferSize>(
                        LuaHandle
                    );
                friend i32 hscript::lua::call_foreign<CallBufferSize>(LuaHandle);
                friend i32 hscript::lua::query_foreign_call<CallBufferSize>(LuaHandle);
                friend i32
                    hscript::lua::get_foreign_call_results<CallBufferSize>(LuaHandle);
                friend i32 hscript::lua::set_manual_command_buffer_pump<CallBufferSize>(
                    LuaHandle
                );
                friend i32 hscript::lua::pump_command_buffer<CallBufferSize>(LuaHandle);

                using _Environment = Environment<HasRPCManager, CallBufferSize>;
                using _Base        = EnvironmentBase<
                    _Environment,
                    LuaContinuableFunction,
                    LuaThreadState,
                    HasRPCManager,
                    CallBufferSize>;
            public:
                Environment() :
                    m_state(nullptr),
                    m_parent(nullptr),
                    m_namespace_depth(0) { /* Empty. */
                }

                ~Environment() { /* Empty. */
                }

                HEMLOCK_NON_COPYABLE(Environment);

                HEMLOCK_MOVABLE(Environment) {
                    this->_Base::operator=(std::move(rhs));

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
                void init(
                    hio::IOManagerBase*                io_manager,
                    EnvironmentRegistry<_Environment>* registry = nullptr,
                    ui32 max_script_length = HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH
                ) final;
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
                void init(
                    _Base*                             parent,
                    hio::IOManagerBase*                io_manager,
                    EnvironmentRegistry<_Environment>* registry = nullptr,
                    ui32 max_script_length = HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH
                ) final;
                /**
                 * @brief Dispose the environment.
                 */
                void dispose() final;

                /**
                 * @brief Load in a script from the provided filepath.
                 *
                 * @param filepath The filepath from which to load the script.
                 */
                bool load(const hio::fs::path& filepath) final;
                /**
                 * @brief Load in the provided script string.
                 *
                 * @param script A string of script to load into the script
                 * environment.
                 */
                bool load(const std::string& script) final;

                /**
                 * @brief Runs everything already loaded into the script environment.
                 */
                bool run() final;
                /**
                 * @brief Loads in the script at the provided filepath, then runs it.
                 *
                 * Anything loaded in before this call will also be ran if no other
                 * calls to run have been made since.
                 *
                 * @param filepath The filepath from which to load the script.
                 */
                bool run(const hio::fs::path& filepath) final;
                /**
                 * @brief Loads in the provided script string, then runs it.
                 *
                 * Anything loaded in before this call will also be ran if no other
                 * calls to run have been made since.
                 *
                 * @param script A string of script to load into the script
                 * environment.
                 */
                bool run(const std::string& script) final;

                /**
                 * @brief Set this environment's current namespace to
                 * the global namespace.
                 */
                void set_global_namespace() final;
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
                void set_namespaces(Strings... namespaces);
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
                void enter_namespaces(Strings... namespaces);

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
                void add_value(std::string_view name, Type val);
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
                    std::string_view name, Delegate<ReturnType(Parameters...)>* delegate
                );
                /**
                 * @brief Add a function to the environment, exposed to the
                 * scripts ran within.
                 *
                 * @tparam Func The invocable type.
                 * @param name The name to expose the function as within the
                 * environment.
                 * @param func The function to be added to the environment.
                 */
                template <typename ReturnType, typename... Parameters>
                void add_c_function(
                    std::string_view name, ReturnType (*func)(Parameters...)
                );
                /**
                 * @brief Add a function to the environment that matches the
                 * lua_CFunction signature, with optional upvalues.
                 *
                 * @tparam Upvalues The types of the optional upvalues.
                 * @param name The name of the function to expose.
                 * @param func The function to expose.
                 * @param upvalues The optional upvalues to bind.
                 */
                template <typename... Upvalues>
                void add_c_function(
                    std::string_view name, i32 (*func)(LuaHandle), Upvalues... upvalues
                );
                /**
                 * @brief Add a closure to the environment, exposed to the
                 * scripts ran within.
                 *
                 * @tparam Func The invocable type.
                 * @param name The name to expose the closure as within the
                 * environment.
                 * @param closure The closure to be added to the environment.
                 */
                template <
                    std::invocable Closure,
                    typename ReturnType,
                    typename... Parameters>
                void add_c_closure(
                    std::string_view name,
                    Closure*         closure,
                    ReturnType (Closure::*func)(Parameters...)
                );
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
                void add_yieldable_c_delegate(
                    std::string_view                                      name,
                    Delegate<YieldableResult<ReturnType>(Parameters...)>* delegate
                );
                /**
                 * @brief Add a function to the environment, exposed to the
                 * scripts ran within.
                 *
                 * @tparam Func The invocable type.
                 * @param name The name to expose the function as within the
                 * environment.
                 * @param func The function to be added to the environment.
                 */
                template <typename ReturnType, typename... Parameters>
                void add_yieldable_c_function(
                    std::string_view name,
                    YieldableResult<ReturnType> (*func)(Parameters...)
                );
                /**
                 * @brief Add a closure to the environment, exposed to the
                 * scripts ran within.
                 *
                 * @tparam Func The invocable type.
                 * @param name The name to expose the closure as within the
                 * environment.
                 * @param closure The closure to be added to the environment.
                 */
                template <
                    std::invocable Closure,
                    typename ReturnType,
                    typename... Parameters>
                void add_yieldable_c_closure(
                    std::string_view name,
                    Closure*         closure,
                    YieldableResult<ReturnType> (Closure::*func)(Parameters...)
                );
                /**
                 * @brief Get a script function from the environment, allowing
                 * calls within C++ into the script. Name can specify namespacing
                 * with dot-separation.
                 *
                 * @tparam ReturnType The return type of the script function.
                 * @tparam Parameters The parameters accepted by the script function.
                 * @param name The name of the script function to obtain.
                 * @param delegate Delegate providing means to call the script
                 * function.
                 * @return True if the script function was obtained, false otherwise.
                 */
                template <typename ReturnType, typename... Parameters>
                bool get_script_function(
                    std::string&& name,
                    OUT ScriptDelegate<ReturnType, Parameters...>& delegate
                );

                /**
                 * @brief Get a continuable script function from the environment,
                 * allowing calls within C++ into the script where the script may yield
                 * back and be continued later.
                 *
                 * @param name The name of the script function to obtain.
                 * @param continuable_function ContinuableFunction object providing
                 * means to call the script function.
                 * @param attached_to_thread If true, creates a thread that the function
                 * is attached to. This thread will be disposed of on function
                 * completion. If false, the function is not attached to a thread,
                 * leaving the attachment up to the caller.
                 * @return True if the script function was obtained, false otherwise.
                 */
                bool get_continuable_script_function(
                    std::string&&               name,
                    OUT LuaContinuableFunction& continuable_function,
                    bool                        attached_to_thread = false
                );

                /**
                 * @brief Creatres a thread context within script environment for
                 * running script components concurrently.
                 *
                 * @return The created thread.
                 */
                LuaThreadState make_thread();

                /**
                 * @brief Destroys the given thread.
                 *
                 * @param thread The thread to destroy.
                 */
                void destroy_thread(LuaThreadState thread);
            protected:
                /**
                 * @brief Pushes namespaces onto the Lua stack. Last
                 * namespace table must be at top of stack for this
                 * to make sense.
                 *
                 * @param _namespace The namespace to push.
                 */
                void push_namespace(const std::string& _namespace);

                /**
                 * @brief Registers a Lua function given a name,
                 * dot-separation for namespacing starting at global.
                 *
                 * @param name The name of the script function to register.
                 * @param state Optional Lua function state struct into which
                 * the registered Lua functions information will be placed.
                 * @return True if the Lua function was registered, false
                 * if it was not already registered and could not be registered.
                 */
                bool register_lua_function(
                    std::string&& name, OUT LuaFunctionState* state = nullptr
                );

                LuaHandle    m_state;
                Environment* m_parent;
                LuaFunctions m_lua_functions;
                LuaFunctions m_lua_continuable_functions;
                i32          m_namespace_depth;
            };
        }  // namespace lua
    }      // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#include "environment.inl"

#endif  // __hemlock_script_lua_environment_h

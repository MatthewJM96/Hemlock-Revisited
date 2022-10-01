#ifndef __hemlock_script_environment_base_hpp
#define __hemlock_script_environment_base_hpp

#ifndef HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH
#define HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH 50 * 1024 * 1024
#endif // HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH

namespace hemlock {
    namespace script {
        template <typename EnvironmentImpl>
        class EnvironmentBase {
        public:
            EnvironmentBase()  { /* Empty. */ }
            ~EnvironmentBase() { /* Empty. */ }

            H_NON_COPYABLE(EnvironmentBase);
            H_MOVABLE(EnvironmentBase) {
                m_max_script_length = rhs.m_max_script_length;

                return *this;
            }

            /**
             * @brief Initialise the environment as a standalone
             * environment.
             *
             * @param max_script_length The maximum length of any
             * script that this environment will process.
             */
            virtual void init(i32 max_script_length = HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH) = 0;
            /**
             * @brief Initialise the environment as a child of a
             * parent environment. All children of the same parent
             * share the same global state.
             *
             * @param parent The parent environment of this one.
             * @param max_script_length The maximum length of any
             * script that this environment will process.
             */
            virtual void init(EnvironmentBase* parent, i32 max_script_length = HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH) = 0;
            /**
             * @brief Dispose the environment.
             */
            virtual void dispose() = 0;

            /**
             * @brief Load in a script from the provided filepath.
             *
             * @param filepath The filepath from which to load the script.
             */
            virtual bool load(const hio::Path& filepath) = 0;
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
             * Anything loaded in before this call will also be ran if no other calls to run have been made since.
             *
             * @param filepath The filepath from which to load the script.
             */
            virtual bool run(const hio::Path& filepath) = 0;
            /**
             * @brief Loads in the provided script string, then runs it.
             *
             * Anything loaded in before this call will also be ran if no other calls to run have been made since.
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
            template <typename ...Strings>
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
            template <typename ...Strings>
            void enter_namespaces(Strings... namespaces) {
                reinterpret_cast<EnvironmentImpl*>(this)->enter_namespaces(namespaces...);
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
            void add_value(const std::string& name, Type val) {
                reinterpret_cast<EnvironmentImpl*>(this)->add_value(name, std::forward<Type>(val));
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
            template <typename ReturnType, typename ...Parameters>
            void add_c_function(const std::string& name, Delegate<ReturnType, Parameters...> func) {
                reinterpret_cast<EnvironmentImpl*>(this)->add_c_function(name, std::move(func));
            }
            /**
             * @brief Get a script function from the environment, allowing
             * calls within C++ into the script.
             *
             * @tparam ReturnType The return type of the script function.
             * @tparam Parameters The parameters accepted by the script function.
             * @param name The name of the script function to obtain.
             * @param do_register Whether to register the obtained function.
             * @return Delegate<ReturnType, Parameters...> Delegate providing
             * means to call the script function.
             */
            template <typename ReturnType, typename ...Parameters>
            Delegate<ReturnType, Parameters...> get_script_function(const std::string& name, bool do_register = true) {
                return reinterpret_cast<EnvironmentImpl*>(this)->get_script_function(name, do_register);
            }
        protected:
            i32 m_max_script_length;
        };
    }
}
namespace hscript = hemlock::script;

#endif // __hemlock_script_environment_base_hpp

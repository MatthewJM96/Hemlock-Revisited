#ifndef __hemlock_script_environment_registry_hpp
#define __hemlock_script_environment_registry_hpp

#include "environment_base.hpp"

namespace hemlock {
    namespace script {
        template <typename Environment>
        using Environments       = std::vector<Environment*>;
        using EnvironmentGroupID = ui32;

        template <typename Environment>
        struct EnvironmentGroup {
            Environment*              parent;
            Environments<Environment> children;
        };

        template <typename Environment>
        using EnvironmentGroups
            = std::unordered_map<EnvironmentGroupID, EnvironmentGroup<Environment>>;
        template <typename Environment>
        using EnvironmentBuilder = Delegate<void(Environment*)>;

        template <typename Environment>
        using EnvironmentRegister = std::unordered_map<std::string, Environment*>;

        template <typename Environment>
        class EnvironmentRegistry {
        public:
            EnvironmentRegistry() { /* Empty. */
            }

            ~EnvironmentRegistry() { /* Empty. */
            }

            HEMLOCK_NON_COPYABLE(EnvironmentRegistry);
            HEMLOCK_MOVABLE(EnvironmentRegistry);

            /**
             * @brief Initialise the environment registry in which script
             * environments and groupings can be created.
             *
             * @param default_io_manager The default IO manager to use for discovering
             * scripts when using load and run functions in registered environments.
             * @param max_script_length The maximum length of any
             * script that this environment will process.
             */
            void init(
                hio::IOManagerBase* default_io_manager = nullptr,
                i32 max_script_length = HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH
            );
            /**
             * @brief Dispose the environment registry.
             */
            void dispose();

            /**
             * @brief Create a group inside which environments can be created.
             * Environments sharing a group share globals, allowing them to
             * communicate with one another.
             *
             * @param builder The builder to use for establishing initial globals,
             * for example functions exposed to the child environments of the group.
             * @return ui32 The ID of the group created.
             */
            EnvironmentGroupID
            create_group(EnvironmentBuilder<Environment>* builder = nullptr);

            /**
             * @brief Create an environment optionally using a builder. This
             * environment will exist outside of any group and so share globals
             * with no other environment.
             *
             * @param name The name of the environment created.
             * @param io_manager The IO manager to give the environment. If this is
             * nullptr, the default IO manager is used instead.
             * @param builder Optional builder to use in creating environment.
             * @return Environment* Pointer to the created environment.
             */
            Environment* create_environment(
                std::string                      name,
                hio::IOManagerBase*              io_manager = nullptr,
                EnvironmentBuilder<Environment>* builder    = nullptr
            );
            /**
             * @brief Create an environment within a group, this environment
             * sharing globals with all others in the group.
             *
             * @param name The name of the environment created.
             * @param group_id The ID of the group to create the environment
             * within.
             * @param io_manager The IO manager to give the environment. If this is
             * nullptr, the default IO manager is used instead.
             * @return Environment* Pointer to the created environment.
             */
            Environment* create_environment(
                std::string         name,
                EnvironmentGroupID  group_id,
                hio::IOManagerBase* io_manager = nullptr
            );

            /**
             * @brief Create environments optionally using a builder. This
             * environment will exist outside of any group and so share globals
             * with no other environment.
             *
             * @param num The number of environments to create.
             * @param names The names of the environments created.
             * @param io_manager The IO manager to give the environment. If this is
             * nullptr, the default IO manager is used instead.
             * @param builder Optional builder to use in creating environment.
             * @return Environment* Pointer to the created environments.
             */
            Environment* create_environments(
                ui32                             num,
                std::string*                     names,
                hio::IOManagerBase*              io_manager = nullptr,
                EnvironmentBuilder<Environment>* builder    = nullptr
            );
            /**
             * @brief Create environments within a group, this environment
             * sharing globals with all others in the group.
             *
             * @param num The number of environments to create.
             * @param names The names of the environments created.
             * @param group_id The ID of the group to create the environment
             * within.
             * @param io_manager The IO manager to give the environment. If this is
             * nullptr, the default IO manager is used instead.
             * @return Environment* Pointer to the created environments.
             */
            Environment* create_environments(
                ui32                num,
                std::string*        names,
                EnvironmentGroupID  group_id,
                hio::IOManagerBase* io_manager = nullptr
            );

            /**
             * Searches through the registry returning an environment with the given
             * name if that environment exists.
             *
             * @param name The name of the envrionment to get.
             * @return Environment* Pointer to the found environment if it exists,
             * nullptr otherwise.
             */
            Environment* get_environment(std::string name);
        protected:
            EnvironmentRegister<Environment> m_register;

            ui32                           m_next_group_id = 0;
            EnvironmentGroups<Environment> m_groups;
            Environments<Environment>      m_ungrouped_envs;

            hio::IOManagerBase* m_default_io_manager;
            i32                 m_max_script_length;
        };
    }  // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#include "environment_registry.inl"

#endif  // __hemlock_script_environment_registry_hpp

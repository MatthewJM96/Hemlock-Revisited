#ifndef __hemlock_script_environment_registry_hpp
#define __hemlock_script_environment_registry_hpp

namespace hemlock {
    namespace script {
        template <typename Environment>
        struct EnvironmentGroup {
            Environment* parent;
            std::vector<Environment*> children;
        };
        template <typename Environment>
        using EnvironmentGroups  = std::unordered_map<ui32, EnvironmentGroup>;
        template <typename Environment>
        using EnvironmentBuilder = Delegate<void, Environment*>;

        template <typename Environment>
        class EnvironmentRegistry {
        public:
            void init();
            void dispose();

            ui32 create_group(EnvironmentBuilder<Environment>* builder = nullptr);
        protected:
            EnvironmentGroups<Environment> m_groups;
        };
    }
}
namespace hscript = hemlock::script;

#endif // __hemlock_script_environment_registry_hpp

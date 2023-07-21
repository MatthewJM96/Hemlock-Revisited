#ifndef __hemlock_mod_scripted_environment_hpp
#define __hemlock_mod_scripted_environment_hpp

#include "script/environment_base.hpp"
#include "script/environment_registry.hpp"

#include "environment.h"

namespace hemlock {
    namespace mod {
        template <typename ScriptEnvironment>
        using ModScriptEnvironmentBuilder
            = hmem::Handle<hscript::EnvironmentBuilder<ScriptEnvironment>>;

        template <typename ScriptEnvironment>
        class ScriptedModEnvironment : public ModEnvironment {
            using _ModScriptEnvironmentBuilder
                = ModScriptEnvironmentBuilder<ScriptEnvironment>;
            using _ModScriptEnvironmentRegistry
                = hscript::EnvironmentRegistry<ScriptEnvironment>;
        public:
            ScriptedModEnvironment();

            virtual ~ScriptedModEnvironment() { /* Empty. */
            }

            void init(
                ModRegistry&&                registry,
                LoadOrder&&                  load_order,
                _ModScriptEnvironmentBuilder script_environment_builder
            );

            void dispose() override final;

            void activate_environment() override final;
            void deactivate_environment() override final;

            void update(FrameTime time) override final;

            bool set_script_environment_builder(_ModScriptEnvironmentBuilder builder);
        protected:
            _ModScriptEnvironmentRegistry m_script_env_registry;
            _ModScriptEnvironmentBuilder  m_script_env_builder;
        };
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#include "scripted_environment.inl"

#endif  // __hemlock_mod_scripted_environment_hpp

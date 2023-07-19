#ifndef __hemlock_mod_scripted_environment_hpp
#define __hemlock_mod_scripted_environment_hpp

#include "environment.h"

namespace hemlock {
    namespace mod {
        template <typename ScriptEnvironment>
        class ScriptedModEnvironment : public ModEnvironment {
        public:
            ScriptedModEnvironment();

            virtual ~ScriptedModEnvironment() { /* Empty. */
            }

            void init(
                ModRegistry&&                   registry,
                LoadOrder&&                     load_order,
                hmem::Handle<ScriptEnvironment> script_environment
            );

            void dispose() override final;

            bool
            set_script_environment(hmem::Handle<ScriptEnvironment> script_environment);

            void update(FrameTime time) override final;
        protected:
            hmem::Handle<ScriptEnvironment> m_script_environment;
        };
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#include "scripted_environment.inl"

#endif  // __hemlock_mod_scripted_environment_hpp

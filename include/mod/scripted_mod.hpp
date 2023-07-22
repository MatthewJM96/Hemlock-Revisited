#ifndef __hemlock_mod_scripted_mod_hpp
#define __hemlock_mod_scripted_mod_hpp

#include "mod.h"

namespace hemlock {
    namespace mod {
        template <typename ScriptEnvironment>
        class ScriptedMod : public Mod {
        public:
            ScriptedMod();
            ~ScriptedMod();

            void update(FrameTime time) override final;
        protected:
            ScriptEnvironment m_script_env;
        };
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_scripted_mod_hpp

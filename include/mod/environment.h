#ifndef __hemlock_mod_environment_h
#define __hemlock_mod_environment_h

#include "io/iomanager.h"

#include "load_order.h"
#include "mod.h"
#include "registry.h"

namespace hemlock {
    namespace mod {
        class ModEnvironment {
        public:
            ModEnvironment();

            virtual ~ModEnvironment() { /* Empty. */
            }

            void         init(ModRegistry&& registry);
            void         init(ModRegistry&& registry, LoadOrder&& load_order);
            virtual void dispose();

            virtual void activate();
            virtual void deactivate();

            virtual void update(FrameTime time);

            bool set_load_order(LoadOrder&& load_order);
        protected:
            ModRegistry m_registry;
            LoadOrder   m_load_order;

            bool m_is_active;

            std::vector<Mod> m_mods;

            io::IOManager* m_base_iom;
        };
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_environment_h

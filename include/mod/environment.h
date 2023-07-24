#ifndef __hemlock_mod_loader_order_h
#define __hemlock_mod_loader_order_h

#include "io/iomanager.h"
#include "io/serialisation.hpp"

#include "mod.h"
#include "registry.h"

namespace hemlock {
    namespace mod {
        // TODO(Matthew): UUID? Can assign a hashable ID at runtime.
        using LoadOrderID = ui64;
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    LoadOrder,
    (_version, ui16),
    (_reserved, ui16),
    (id, hmod::LoadOrderID),
    (name, std::string),
    (mods, std::vector<hmod::ModID>),
    (description, std::string)
    // Last updated?
    // Version?
)

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

#endif  // __hemlock_mod_loader_order_h

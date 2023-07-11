#ifndef __hemlock_mod_loader_order_h
#define __hemlock_mod_loader_order_h

#include "io/ordered_iomanager.h"

#include "mod.h"

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
    (id, LoadOrderID),
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
            ~ModEnvironment();

            void update(FrameTime time);

            bool set_load_order(const LoadOrder& load_order);
        protected:
            std::vector<Mod> m_mods;

            io::OrderedIOManager m_non_merging_iom;
        }
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_loader_order_h

#ifndef __hemlock_mod_load_order_validate_order_h
#define __hemlock_mod_load_order_validate_order_h

#include "mod/load_order/load_order.h"
#include "mod/registry.h"

namespace hemlock {
    namespace mod {
        /**
         * @brief Validates the depends conditions of the current mod by checking
         * against all preceding mods in the load order for completion of hard_depends,
         * and all subsequent mods in the load order for lack of any soft_depends. It is
         * assumed that all metadata are available in the registry.
         *
         * @param index The index of the mod to validate the depends conditions of.
         * @param load_order The load order in which the mod resides.
         * @param registry The registry to refer to for mod metadata.
         * @return True if the mod being checked has its dependencies validated. False
         * otherwise.
         */
        bool validate_depends(
            size_t                   index,
            const hmod::LoadOrder&   load_order,
            const hmod::ModRegistry& registry
        );

        /**
         * @brief Validates the wanted-by conditions of the current mod by checking
         * against all preceding mods in the load order for lack of soft_wanted_by, and
         * all subsequent mods in the load order for completion of hard_wanted_by. It is
         * assumed that all metadata are available in the registry.
         *
         * @param index The index of the mod to validate the wanted-by conditions of.
         * @param load_order The load order in which the mod resides.
         * @param registry The registry to refer to for mod metadata.
         * @return True if the mod being checked has its dependencies validated. False
         * otherwise.
         */
        bool validate_wanted_by(
            size_t                   index,
            const hmod::LoadOrder&   load_order,
            const hmod::ModRegistry& registry
        );
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_load_order_validate_order_h

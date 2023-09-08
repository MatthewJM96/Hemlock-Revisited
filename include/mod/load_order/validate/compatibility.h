#ifndef __hemlock_mod_load_order_validate_compatibility_h
#define __hemlock_mod_load_order_validate_compatibility_h

#include "mod/load_order/state.h"
#include "mod/registry.h"

namespace hemlock {
    namespace mod {
        /**
         * @brief Validates that a given mod's compatibility conditions are satisfied.
         *
         * @param index The index of the mod to validate the compatibility conditions
         * of.
         * @param load_order The load order in which the mod resides.
         * @param registry The registry to refer to for mod metadata.
         * @return LoadOrderState::COMPATIBLE if the mod's compatibility conditions were
         * satisfied, LoadOrderState::INCOMPATIBLE if an explicit incompatibility is
         * found, LoadOrderState::VERSION_MISMATCH if there is an explicit compatibility
         * with a mod for which the version in the load order is different to the
         * compatible version range.
         */
        LoadOrderState is_compatible(
            size_t index, const LoadOrder& load_order, const ModRegistry& registry
        );
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_load_order_validate_compatibility_h

#ifndef __hemlock_mod_loader_order_h
#define __hemlock_mod_loader_order_h

#include "mod/metadata.h"
#include "mod/registry.h"

#include "state.h"

namespace hemlock {
    namespace mod {
        /**
         * @brief Validates the provided load order, checking that each mod's depends,
         * wanted-by and compatibility conditions are satisfied.
         *
         * @param load_order The load order to validate.
         * @param registry The registry to refer to for mod metadata.
         * @return LoadOrderState::VALID_ORDER if the load order is valid,
         * LoadOrderState::INVALID_ORDER if the load order is invalid,
         * LoadOrderState::MISSING_MOD_METADATA if some mod in the load order is missing
         * the metadata necessary to validate the load order.
         */
        LoadOrderState
        validate_load_order(const LoadOrder& load_order, const ModRegistry& registry);

        /**
         * @brief Modifies the load order such that it satisfies each mod's depends,
         * wanted-by and compatibility conditions. If the load order is already valid
         * or cannot be made valid, no change is made.
         *
         * @param load_order The load order to make valid.
         * @param registry The registry to refer to for mod metadata.
         * @return LoadOrderState::VALID_ORDER if the load order is valid after exit of
         * this function, LoadOrderState::INVALID_ORDER if the load order is invalid at
         * exit of this function, LoadOrderState::MISSING_METADATA if some mod in
         * the load order is missing the metadata necessary to make the load order
         * valid.
         */
        LoadOrderState
        make_load_order_valid(LoadOrder& load_order, const ModRegistry& registry);
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_loader_order_h

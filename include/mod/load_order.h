#ifndef __hemlock_mod_loader_order_h
#define __hemlock_mod_loader_order_h

#include "io/serialisation.hpp"

#include "metadata.h"
#include "registry.h"

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    LoadOrder,
    (_version, ui16),
    (_reserved, ui16),
    (id, boost::uuids::uuid),
    (name, std::string),
    (mods, std::vector<boost::uuids::uuid>),
    (description, std::string),
    (version, hemlock::SemanticVersion),
    (last_updated, std::chrono::sys_seconds)
)

H_DECL_ENUM_WITH_SERIALISATION(
    hemlock::mod, LoadOrderState, ui8, VALID_ORDER, INVALID_ORDER, MISSING_MOD_METADATA
)

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
         * exit of this function, LoadOrderState::MISSING_MOD_METADATA if some mod in
         * the load order is missing the metadata necessary to make the load order
         * valid.
         */
        LoadOrderState
        make_load_order_valid(LoadOrder& load_order, const ModRegistry& registry);
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_loader_order_h

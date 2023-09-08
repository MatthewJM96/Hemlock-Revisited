#ifndef __hemlock_mod_loader_order_state_h
#define __hemlock_mod_loader_order_state_h

#include "io/serialisation.hpp"

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

/**
 * @brief Indicates the validity of a load order:
 *          - VALID indicates a load order satisfying all conditions of contained mods;
 *          - COMPATIBLE indicates a load order satisfying compatibility conditions but
 *              perhaps not any other conditions of the contained mods;
 *          - VERSION_MISMATCH indicates a load order for which at least one mod states
 *              an explicit compatibility with another mod for a version other than that
 *              which is present in the load order;
 *          - INCOMPATIBLE indicates a load order for which at least one mod states an
 *              explicit incompatibility with another mod for the version that is
 *              present in the load order;
 *          - INVALID_ORDER indicates a load order for which at least one mod's
 *              dependency and wanted-by conditions are not wholly met.
 *          - MISSING_MOD_METADATA indicates a load order for which at least one mod
 *              is missing metadata - likely the mod is missing or malformed.
 */
H_DECL_ENUM_WITH_SERIALISATION(
    hemlock::mod,
    LoadOrderState,
    ui8,
    VALID,
    COMPATIBLE,
    VERSION_MISMATCH,
    INCOMPATIBLE,
    INVALID_ORDER,
    MISSING_METADATA,
)

#endif  // __hemlock_mod_loader_order_state_h

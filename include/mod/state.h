#ifndef __hemlock_mod_state_h
#define __hemlock_mod_state_h

#include "io/serialisation.hpp"

namespace hemlock {
    namespace mod {
        using ModCompatibilities
            = std::unordered_map<UUID, std::optional<hemlock::Versions>>;
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModMetadata,
    (id, hemlock::UUID),
    (name, std::string),
    (authors, std::vector<std::string>),
    (description, std::string),
    (version, hemlock::SemanticVersion),
    (last_updated, std::chrono::sys_seconds),
    (hard_depends, std::optional<hmod::ModCompatibilities>),
    (soft_depends, std::optional<hmod::ModCompatibilities>),
    (hard_wanted_by, std::optional<hmod::ModCompatibilities>),
    (soft_wanted_by, std::optional<hmod::ModCompatibilities>),
    (compatible, std::optional<hmod::ModCompatibilities>),
    (incompatible, std::optional<hmod::ModCompatibilities>)
)

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    LoadOrder,
    (id, hemlock::UUID),
    (name, std::string),
    (mods, std::vector<hemlock::UUID>),
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
 *              dependency and wanted-by conditions are not wholly met;
 *          - INCONSISTENT indicates a load order for which at least one mod has
 *              requirements that cannot be satisfied under any condition (e.g. lists
 *              the same mod as a depends and a wanted-by);
 *          - MISSING_METADATA indicates a load order for which at least one mod
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
    INCONSISTENT,
    MISSING_METADATA
)

namespace hemlock {
    namespace mod {
        using ModRegistry       = std::unordered_map<UUID, ModMetadata>;
        using LoadOrderRegistry = std::unordered_map<UUID, LoadOrder>;

        using ModDirectories = std::vector<hio::fs::path>;
    }  // namespace mod
}  // namespace hemlock

#endif  // __hemlock_mod_state_h

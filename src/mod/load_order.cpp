#include "stdafx.h"

#include "mod/load_order.h"
#include "mod/metadata.h"

H_DEF_STRUCT_WITH_SERIALISATION(
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

H_DEF_ENUM_WITH_SERIALISATION(hemlock::mod, LoadOrderState)

/**
 * Validates the depends, wanted-by, and compatibility conditions,
 * return true if they are valid, false otherwise.
 */
static bool validate_conditions(const hmod::ModMetadata& metadata) {
    // Verify no depends is marked also as a wanted_by.

    if (metadata.hard_depends.has_value()) {
        for (const auto& [hard_depends_id, _] : metadata.hard_depends.value()) {
            if (metadata.hard_wanted_by.has_value()) {
                for (const auto& [hard_wanted_by_id, _2] :
                     metadata.hard_wanted_by.value())
                {
                    if (hard_depends_id == hard_wanted_by_id) {
                        return false;
                    }
                }
            }

            if (metadata.soft_wanted_by.has_value()) {
                for (const auto& [soft_wanted_by_id, _2] :
                     metadata.soft_wanted_by.value())
                {
                    if (hard_depends_id == soft_wanted_by_id) {
                        return false;
                    }
                }
            }
        }
    }

    if (metadata.soft_depends.has_value()) {
        for (const auto& [soft_depends_id, _] : metadata.hard_depends.value()) {
            if (metadata.hard_wanted_by.has_value()) {
                for (const auto& [hard_wanted_by_id, _2] :
                     metadata.hard_wanted_by.value())
                {
                    if (soft_depends_id == hard_wanted_by_id) {
                        return false;
                    }
                }
            }

            if (metadata.soft_wanted_by.has_value()) {
                for (const auto& [soft_wanted_by_id, _2] :
                     metadata.soft_wanted_by.value())
                {
                    if (soft_depends_id == soft_wanted_by_id) {
                        return false;
                    }
                }
            }
        }
    }

    // Verify no compatible is marked also as incompatible for overlapping versions.

    if (metadata.compatible.has_value()) {
        for (const auto& [compatible_id, compatible_versions] :
             metadata.compatible.value())
        {
            if (metadata.incompatible.has_value()) {
                for (const auto& [incompatible_id, incompatible_versions] :
                     metadata.incompatible.value())
                {
                    if ((compatible_id == incompatible_id)) {
                        if (!compatible_versions.has_value()
                            || !incompatible_versions.has_value())
                            return false;

                        if (overlaps(
                                compatible_versions.value(),
                                incompatible_versions.value()
                            ))
                            return false;
                    }
                }
            }
        }
    }

    return true;
}

/**
 * Validates the depends conditions of the current mod by checking against
 * all preceding mods in the load order for completion of hard_depends, and
 * all subsequent mods in the load order for lack of any soft_depends. It is
 * assumed that all metadata are available in the registry.
 */
static bool validate_depends(
    size_t                   curr_index,
    const hmod::ModMetadata& curr_metadata,
    const hmod::LoadOrder&   load_order,
    const hmod::ModRegistry& registry
) {
    if (curr_metadata.hard_depends.has_value()) {
        for (const auto& [hard_depends_id, hard_depends_version] :
             curr_metadata.hard_depends.value())
        {
            bool satisfied_condition = false;

            for (size_t before_idx = 0; before_idx < curr_index; ++before_idx) {
                const auto& before_id = load_order.mods[before_idx];

                const hmod::ModMetadata& before_metadata = registry.at(before_id);

                if ((before_id == hard_depends_id) && hard_depends_version.has_value()
                    && overlaps(before_metadata.version, hard_depends_version.value()))
                {
                    satisfied_condition = true;
                    break;
                }
            }

            if (!satisfied_condition) return false;
        }
    }

    if (curr_metadata.soft_depends.has_value()) {
        for (const auto& [soft_depends_id, _] : curr_metadata.soft_depends.value()) {
            for (size_t after_idx = curr_index + 1; after_idx < load_order.mods.size();
                 ++after_idx)
            {
                const auto& after_id = load_order.mods[after_idx];

                if (after_id == soft_depends_id) return false;
            }
        }
    }

    return true;
}

/**
 * Validates the wanted-by conditions of the current mod by checking against
 * all preceding mods in the load order for lack of soft_wanted_by, and
 * all subsequent mods in the load order for completion of hard_wanted_by. It is
 * assumed that all metadata are available in the registry.
 */
static bool validate_wanted_by(
    size_t                   curr_index,
    const hmod::ModMetadata& curr_metadata,
    const hmod::LoadOrder&   load_order,
    const hmod::ModRegistry& registry
) {
    if (curr_metadata.hard_wanted_by.has_value()) {
        for (const auto& [hard_wanted_by_id, hard_wanted_by_version] :
             curr_metadata.hard_wanted_by.value())
        {
            bool satisfied_condition = false;

            for (size_t after_idx = curr_index + 1; after_idx < load_order.mods.size();
                 ++after_idx)
            {
                const auto& after_id = load_order.mods[after_idx];

                const hmod::ModMetadata& after_metadata = registry.at(after_id);

                if ((after_id == hard_wanted_by_id)
                    && hard_wanted_by_version.has_value()
                    && overlaps(after_metadata.version, hard_wanted_by_version.value()))
                {
                    satisfied_condition = true;
                    break;
                }
            }

            if (!satisfied_condition) return false;
        }
    }

    if (curr_metadata.soft_wanted_by.has_value()) {
        for (const auto& [soft_wanted_by_id, _] : curr_metadata.soft_wanted_by.value())
        {
            for (size_t before_idx = 0; before_idx < curr_index; ++before_idx) {
                const auto& before_id = load_order.mods[before_idx];

                if (before_id == soft_wanted_by_id) return false;
            }
        }
    }

    return true;
}

/**
 * Validates the compatibility conditions of the current mod by checking against
 * all preceding and subsequent mods in the load order for lack of versions matching
 * listed incompatibilities. Where a mod has listed compatible ranges,
 * LoadOrderState::MOD_COMPATIBILITY_VERSION_MISMATCH is returned, where an incompatible
 * version is encountered LoadOrderState::INVALID_ORDER is returned, otherwise
 * LoadOrderState::VALID_ORDER is returned.
 */
static hmod::LoadOrderState validate_compatibilities(
    size_t                   curr_index,
    const hmod::ModMetadata& curr_metadata,
    const hmod::LoadOrder&   load_order,
    const hmod::ModRegistry& registry
) {
    if (!curr_metadata.compatible.has_value()
        && !curr_metadata.incompatible.has_value())
        return hmod::LoadOrderState::VALID_ORDER;

    for (size_t other_idx = 0; other_idx < load_order.mods.size(); ++other_idx) {
        if (other_idx == curr_index) continue;

        const auto& other_id = load_order.mods[other_idx];

        const hmod::ModMetadata& other_metadata = registry.at(other_id);

        if (curr_metadata.incompatible.has_value()) {
            for (const auto& [incompatible_id, incompatible_versions] :
                 curr_metadata.incompatible.value())
            {
                if (other_id == incompatible_id) {
                    if (incompatible_versions.has_value()) {
                        if (overlaps(
                                other_metadata.version, incompatible_versions.value()
                            ))
                            return hmod::LoadOrderState::INVALID_ORDER;
                    } else {
                        return hmod::LoadOrderState::INVALID_ORDER;
                    }
                }
            }
        }

        if (curr_metadata.compatible.has_value()) {
            for (const auto& [compatible_id, compatible_versions] :
                 curr_metadata.compatible.value())
            {
                if (other_id == compatible_id) {
                    if (compatible_versions.has_value()
                        && !overlaps(
                            other_metadata.version, compatible_versions.value()
                        ))
                        return hmod::LoadOrderState::MOD_COMPATIBILITY_VERSION_MISMATCH;
                }
            }
        }
    }

    return hmod::LoadOrderState::VALID_ORDER;
}

hmod::LoadOrderState
hmod::validate_load_order(const LoadOrder& load_order, const ModRegistry& registry) {
    // Not efficient but first do a pass to check if we're missing any metadata needed
    // to validate the load order.

    try {
        for (const auto& id : load_order.mods) {
            registry.at(id);
        }
    } catch (std::out_of_range&) {
        return LoadOrderState::MISSING_MOD_METADATA;
    }

    // We now know all metadata we need does indeed exist, so we will iterate each mod
    // in the load order now, and check its conditions.

    LoadOrderState curr_state = LoadOrderState::VALID_ORDER;

    for (size_t curr_idx = 0; curr_idx < load_order.mods.size(); ++curr_idx) {
        const auto& curr_id = load_order.mods[curr_idx];

        const ModMetadata& curr_metadata = registry.at(curr_id);

        if (!validate_conditions(curr_metadata)) {
            return LoadOrderState::INVALID_ORDER;
        }

        if (!validate_depends(curr_idx, curr_metadata, load_order, registry)) {
            return LoadOrderState::INVALID_ORDER;
        }

        if (!validate_wanted_by(curr_idx, curr_metadata, load_order, registry)) {
            return LoadOrderState::INVALID_ORDER;
        }

        auto comp_state
            = validate_compatibilities(curr_idx, curr_metadata, load_order, registry);
        if (comp_state == LoadOrderState::INVALID_ORDER) {
            return LoadOrderState::INVALID_ORDER;
        } else if (comp_state == LoadOrderState::MOD_COMPATIBILITY_VERSION_MISMATCH) {
            curr_state = LoadOrderState::MOD_COMPATIBILITY_VERSION_MISMATCH;
        }
    }

    return curr_state;
}

hmod::LoadOrderState
hmod::make_load_order_valid(LoadOrder& load_order, const ModRegistry& registry) { }

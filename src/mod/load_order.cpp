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

static bool validate_depends(
    size_t                   curr_index,
    const hmod::ModMetadata& curr_metadata,
    const hmod::ModRegistry& registry
) { }

static bool validate_wanted_by(
    size_t                   curr_index,
    const hmod::ModMetadata& curr_metadata,
    const hmod::ModRegistry& registry
) { }

static bool validate_compatibilities(
    size_t                   curr_index,
    const hmod::ModMetadata& curr_metadata,
    const hmod::ModRegistry& registry
) { }

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

    for (size_t curr_idx = 0; curr_idx < load_order.mods.size(); ++curr_idx) {
        const auto& curr_id = load_order.mods[curr_idx];

        const ModMetadata& curr_metadata = registry.at(curr_id);

        if (!validate_conditions(curr_metadata)) {
            return LoadOrderState::INVALID_ORDER;
        }

        if (!validate_depends(curr_idx, curr_metadata, registry)) {
            return LoadOrderState::INVALID_ORDER;
        }

        if (!validate_wanted_by(curr_idx, curr_metadata, registry)) {
            return LoadOrderState::INVALID_ORDER;
        }

        if (!validate_compatibilities(curr_idx, curr_metadata, registry)) {
            return LoadOrderState::INVALID_ORDER;
        }
    }

    return LoadOrderState::VALID_ORDER;
}

hmod::LoadOrderState
hmod::make_load_order_valid(LoadOrder& load_order, const ModRegistry& registry) { }

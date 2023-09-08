#include "stdafx.h"

#include "mod/load_order/validate/compatibility.h"

static hmod::LoadOrderState
__mods_compatible(const hmod::ModMetadata& first, const hmod::ModMetadata& second) {
    if (first.incompatible.has_value()) {
        for (const auto& [incompatible_id, incompatible_versions] :
             first.incompatible.value())
        {
            if (second.id == incompatible_id) {
                if (incompatible_versions.has_value()) {
                    if (overlaps(second.version, incompatible_versions.value()))
                        return hmod::LoadOrderState::INCOMPATIBLE;
                } else {
                    return hmod::LoadOrderState::INCOMPATIBLE;
                }
            }
        }
    }

    if (first.compatible.has_value()) {
        for (const auto& [compatible_id, compatible_versions] :
             first.compatible.value())
        {
            if (second.id == compatible_id) {
                if (compatible_versions.has_value()
                    && !overlaps(second.version, compatible_versions.value()))
                    return hmod::LoadOrderState::VERSION_MISMATCH;
            }
        }
    }

    return hmod::LoadOrderState::COMPATIBLE;
}

static hmod::LoadOrderState
mods_compatible(const hmod::ModMetadata& first, const hmod::ModMetadata& second) {
    auto first_comp  = __mods_compatible(first, second);
    auto second_comp = __mods_compatible(second, first);

    if (first_comp < second_comp) {
        return second_comp;
    }

    return first_comp;
}

hmod::LoadOrderState hmod::is_compatible(
    size_t index, const LoadOrder& load_order, const ModRegistry& registry
) {
    const auto& id = load_order.mods[index];

    const ModMetadata& metadata = registry.at(id);

    // If this mod has no stated compatibilities or incompatibilities then it is by
    // definition compatible to the best of our knowledge.
    if (!metadata.compatible.has_value() && !metadata.incompatible.has_value())
        return LoadOrderState::COMPATIBLE;

    // Iterate all other mods and determine if they satisfy the compatibility conditions
    // of the currently considered mod.
    for (size_t other_idx = 0; other_idx < load_order.mods.size(); ++other_idx) {
        if (other_idx == index) continue;

        const auto& other_id = load_order.mods[other_idx];

        const ModMetadata& other_metadata = registry.at(other_id);

        LoadOrderState compatibility = mods_compatible(metadata, other_metadata);

        if (compatibility != LoadOrderState::COMPATIBLE) return compatibility;
    }

    return LoadOrderState::COMPATIBLE;
}

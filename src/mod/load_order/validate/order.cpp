#include "stdafx.h"

#include "mod/load_order/validate/order.h"

bool hmod::validate_depends(
    size_t index, const hmod::LoadOrder& load_order, const hmod::ModRegistry& registry
) {
    const auto& id = load_order.mods[index];

    const ModMetadata& metadata = registry.at(id);

    if (metadata.hard_depends.has_value()) {
        for (const auto& [hard_depends_id, hard_depends_version] :
             metadata.hard_depends.value())
        {
            bool satisfied_condition = false;

            for (size_t before_idx = 0; before_idx < index; ++before_idx) {
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

    if (metadata.soft_depends.has_value()) {
        for (const auto& [soft_depends_id, _] : metadata.soft_depends.value()) {
            for (size_t after_idx = index + 1; after_idx < load_order.mods.size();
                 ++after_idx)
            {
                const auto& after_id = load_order.mods[after_idx];

                if (after_id == soft_depends_id) return false;
            }
        }
    }

    return true;
}

bool hmod::validate_wanted_by(
    size_t index, const hmod::LoadOrder& load_order, const hmod::ModRegistry& registry
) {
    const auto& id = load_order.mods[index];

    const ModMetadata& metadata = registry.at(id);

    if (metadata.hard_wanted_by.has_value()) {
        for (const auto& [hard_wanted_by_id, hard_wanted_by_version] :
             metadata.hard_wanted_by.value())
        {
            bool satisfied_condition = false;

            for (size_t after_idx = index + 1; after_idx < load_order.mods.size();
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

    if (metadata.soft_wanted_by.has_value()) {
        for (const auto& [soft_wanted_by_id, _] : metadata.soft_wanted_by.value()) {
            for (size_t before_idx = 0; before_idx < index; ++before_idx) {
                const auto& before_id = load_order.mods[before_idx];

                if (before_id == soft_wanted_by_id) return false;
            }
        }
    }

    return true;
}

#include "stdafx.h"

#include "mod/load_order/validate/consistency.h"

bool hmod::is_self_consistent(const hmod::ModMetadata& metadata) {
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

#ifndef __hemlock_version_validate_h
#define __hemlock_version_validate_h

#include "semver.h"

H_DECL_UNION_WITH_SERIALISATION(
    hemlock,
    Versions,
    ui8,
    (H_NON_POD_TYPE, LIST, (list, hemlock::VersionList)),
    (H_NON_POD_TYPE, RANGE, (range, hemlock::VersionRange)),
    (H_NON_POD_TYPE, MINIMUM, (minimum, hemlock::VersionMinimum)),
    (H_NON_POD_TYPE, MAXIMUM, (maximum, hemlock::VersionMaximum))
)

namespace hemlock {
    /**
     * @brief Determines if two version ranges overlap in anyway.
     *
     * @param a The first version range to consider.
     * @param b The second version range to consider.
     * @return True if any part of the two ranges overlap, false otherwise.
     */
    bool overlaps(Versions a, Versions b);

    /**
     * @brief Determines if two version ranges overlap in anyway.
     *
     * @param a The first version range to consider.
     * @param b The second version range to consider.
     * @return True if any part of the two ranges overlap, false otherwise.
     */
    bool overlaps(SemanticVersion a, Versions b);

    /**
     * @brief Determines if two version ranges overlap in anyway.
     *
     * @param a The first version range to consider.
     * @param b The second version range to consider.
     * @return True if any part of the two ranges overlap, false otherwise.
     */
    bool overlaps(Versions a, SemanticVersion b);
}  // namespace hemlock

#endif  // __hemlock_version_validate_h

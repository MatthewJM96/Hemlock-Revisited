#ifndef __hemlock_version_validate_h
#define __hemlock_version_validate_h

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock, VersionedFormat, (_version, ui16), (_reserved, ui16)
)

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock, VersionList, (versions, std::vector<hemlock::SemanticVersion>),
)

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock,
    VersionRange,
    (minimum, hemlock::SemanticVersion),
    (maximum, hemlock::SemanticVersion)
)

namespace hemlock {
    struct VersionMinimum : public SemanticVersion { };

    struct VersionMaximum : public SemanticVersion { };
}  // namespace hemlock

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
}  // namespace hemlock

#endif  // __hemlock_version_validate_h

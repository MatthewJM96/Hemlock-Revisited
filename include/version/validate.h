#ifndef __hemlock_version_validate_h
#define __hemlock_version_validate_h

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock, VersionList, (versions, std::vector<hemlock::SemanticVersion>),
)

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock,
    VersionRange,
    (minimum, hemlock::SemanticVersion),
    (maximum, hemlock::SemanticVersion)
)

H_DECL_UNION_WITH_SERIALISATION(
    hemlock::mod,
    Versions,
    ui8,
    (H_NON_POD_TYPE(), LIST, (list, hemlock::VersionList)),
    (H_NON_POD_TYPE(), RANGE, (range, hemlock::VersionRange)),
    (H_NON_POD_TYPE(), MINIMUM, (minimum, hemlock::SemanticVersion)),
    (H_NON_POD_TYPE(), MAXIMUM, (maximum, hemlock::SemanticVersion)),
)

#endif  // __hemlock_version_validate_h

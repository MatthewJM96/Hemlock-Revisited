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

#endif  // __hemlock_version_validate_h

#include "stdafx.h"

#include "version/validate.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock, VersionList, (versions, std::vector<hemlock::SemanticVersion>),
)

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock,
    VersionRange,
    (minimum, hemlock::SemanticVersion),
    (maximum, hemlock::SemanticVersion)
)

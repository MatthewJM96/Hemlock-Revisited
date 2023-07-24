#include "stdafx.h"

#include "version/validate.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock, VersionedFormat, (_version, ui16), (_reserved, ui16)
)

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock, VersionList, (versions, std::vector<hemlock::SemanticVersion>),
)

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock,
    VersionRange,
    (minimum, hemlock::SemanticVersion),
    (maximum, hemlock::SemanticVersion)
)

H_DEF_UNION_WITH_SERIALISATION(
    hemlock,
    Versions,
    ui8,
    (H_NON_POD_TYPE(), LIST, (list, hemlock::VersionList)),
    (H_NON_POD_TYPE(), RANGE, (range, hemlock::VersionRange)),
    (H_NON_POD_TYPE(), MINIMUM, (minimum, hemlock::SemanticVersion)),
    (H_NON_POD_TYPE(), MAXIMUM, (maximum, hemlock::SemanticVersion)),
)

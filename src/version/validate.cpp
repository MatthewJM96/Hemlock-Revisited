#include "stdafx.h"

#include "version/semver.h"
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
    (H_NON_POD_TYPE, LIST, (list, hemlock::VersionList)),
    (H_NON_POD_TYPE, RANGE, (range, hemlock::VersionRange)),
    (H_NON_POD_TYPE, MINIMUM, (minimum, hemlock::VersionMinimum)),
    (H_NON_POD_TYPE, MAXIMUM, (maximum, hemlock::VersionMaximum))
)

static bool lists_overlap(hemlock::Versions a, hemlock::Versions b) {
    for (const auto& a_ver : a.list.versions) {
        for (const auto& b_ver : b.list.versions) {
            if (a_ver == b_ver) return true;
        }
    }
    return false;
}

static bool list_and_range_overlap(hemlock::Versions a, hemlock::Versions b) {
    for (const auto& a_ver : a.list.versions) {
        if ((a_ver > b.range.minimum) && (a_ver < b.range.maximum)) return true;
    }
    return false;
}

static bool list_and_minimum_overlap(hemlock::Versions a, hemlock::Versions b) {
    for (const auto& a_ver : a.list.versions) {
        if (a_ver > b.minimum) return true;
    }
    return false;
}

static bool list_and_maximum_overlap(hemlock::Versions a, hemlock::Versions b) {
    for (const auto& a_ver : a.list.versions) {
        if (a_ver < b.maximum) return true;
    }
    return false;
}

static bool ranges_overlap(hemlock::Versions a, hemlock::Versions b) {
    if ((a.range.minimum <= b.range.maximum) || (a.range.maximum >= b.range.minimum))
        return true;
    return false;
}

static bool range_and_minimum_overlap(hemlock::Versions a, hemlock::Versions b) {
    if (a.range.maximum >= b.minimum) return true;
    return false;
}

static bool range_and_maximum_overlap(hemlock::Versions a, hemlock::Versions b) {
    if (a.range.minimum <= b.maximum) return true;
    return false;
}

static bool minimum_maximum_overlap(hemlock::Versions a, hemlock::Versions b) {
    if (a.minimum <= b.maximum) return true;
    return false;
}

bool hemlock::overlaps(Versions a, Versions b) {
    switch (a.kind) {
        case VersionsKind::LIST:
            switch (b.kind) {
                case VersionsKind::LIST:
                    return lists_overlap(a, b);
                case VersionsKind::RANGE:
                    return list_and_range_overlap(a, b);
                case VersionsKind::MINIMUM:
                    return list_and_minimum_overlap(a, b);
                case VersionsKind::MAXIMUM:
                    return list_and_maximum_overlap(a, b);
                default:
                    break;
            }
            break;
        case VersionsKind::RANGE:
            switch (b.kind) {
                case VersionsKind::LIST:
                    return list_and_range_overlap(b, a);
                case VersionsKind::RANGE:
                    return ranges_overlap(a, b);
                case VersionsKind::MINIMUM:
                    return range_and_minimum_overlap(a, b);
                case VersionsKind::MAXIMUM:
                    return range_and_maximum_overlap(a, b);
                default:
                    break;
            }
        case VersionsKind::MINIMUM:
            switch (b.kind) {
                case VersionsKind::LIST:
                    return list_and_minimum_overlap(b, a);
                case VersionsKind::RANGE:
                    return range_and_minimum_overlap(b, a);
                case VersionsKind::MINIMUM:
                    return true;
                case VersionsKind::MAXIMUM:
                    return minimum_maximum_overlap(a, b);
                default:
                    break;
            }
        case VersionsKind::MAXIMUM:
            switch (b.kind) {
                case VersionsKind::LIST:
                    return list_and_maximum_overlap(b, a);
                case VersionsKind::RANGE:
                    return range_and_maximum_overlap(b, a);
                case VersionsKind::MINIMUM:
                    return minimum_maximum_overlap(b, a);
                case VersionsKind::MAXIMUM:
                    return true;
                default:
                    break;
            }
        default:
            break;
    }

    return false;
}

bool hemlock::overlaps(SemanticVersion a, Versions b) {
    Versions vers;
    vers.kind = VersionsKind::LIST;
    vers.list = { { a } };

    return overlaps(vers, b);
}

bool hemlock::overlaps(Versions a, SemanticVersion b) {
    return overlaps(b, a);
}

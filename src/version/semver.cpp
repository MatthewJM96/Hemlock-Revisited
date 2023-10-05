#include "stdafx.h"

#include "version/semver.h"

hemlock::SemanticVersion::SemanticVersion() :
    m_major(0), m_minor(0), m_patch(0), m_pre_release(nullptr), m_build(nullptr) {
    // Empty.
}

hemlock::SemanticVersion::SemanticVersion(ui32 major, ui32 minor, ui32 patch) :
    m_major(major),
    m_minor(minor),
    m_patch(patch),
    m_pre_release(nullptr),
    m_build(nullptr) {
    // Empty.
}

hemlock::SemanticVersion::SemanticVersion(
    ui32 major, ui32 minor, ui32 patch, const char* pre_release
) :
    m_major(major),
    m_minor(minor),
    m_patch(patch),
    m_pre_release(pre_release),
    m_build(nullptr) {
    // Empty.
}

hemlock::SemanticVersion::SemanticVersion(
    ui32 major, ui32 minor, ui32 patch, const char* pre_release, const char* build
) :
    m_major(major),
    m_minor(minor),
    m_patch(patch),
    m_pre_release(pre_release),
    m_build(build) {
    // Empty.
}

hemlock::SemanticVersion::SemanticVersion(const SemanticVersion& ver) :
    m_major(ver.m_major),
    m_minor(ver.m_minor),
    m_patch(ver.m_patch),
    m_pre_release(ver.m_pre_release),
    m_build(ver.m_build) {
    // Empty.
}

hemlock::SemanticVersion::SemanticVersion(SemanticVersion&& ver) :
    m_major(std::move(ver.m_major)),
    m_minor(std::move(ver.m_minor)),
    m_patch(std::move(ver.m_patch)),
    m_pre_release(std::move(ver.m_pre_release)),
    m_build(std::move(ver.m_build)) {
    // Empty.
}

hemlock::SemanticVersion::~SemanticVersion() {
    if (m_pre_release) delete[] m_pre_release;
    if (m_build) delete[] m_build;
}

hemlock::SemanticVersion& hemlock::SemanticVersion::operator=(const SemanticVersion& ver
) {
    m_major       = ver.m_major;
    m_minor       = ver.m_minor;
    m_patch       = ver.m_patch;
    m_pre_release = ver.m_pre_release;
    m_build       = ver.m_build;

    return *this;
}

hemlock::SemanticVersion& hemlock::SemanticVersion::operator=(SemanticVersion&& ver) {
    m_major       = std::move(ver.m_major);
    m_minor       = std::move(ver.m_minor);
    m_patch       = std::move(ver.m_patch);
    m_pre_release = std::move(ver.m_pre_release);
    m_build       = std::move(ver.m_build);

    return *this;
}

bool hemlock::SemanticVersion::operator==(const SemanticVersion& rhs) const {
    return (m_major == rhs.m_major) && (m_minor == rhs.m_minor)
           && (m_patch == rhs.m_patch) && (m_pre_release == rhs.m_pre_release)
           && (m_build == rhs.m_build);
}

std::strong_ordering hemlock::SemanticVersion::operator<=>(const SemanticVersion& rhs
) const {
    if (m_major > rhs.m_major) {
        return std::strong_ordering::greater;
    } else if (m_major < rhs.m_major) {
        return std::strong_ordering::less;
    }

    if (m_minor > rhs.m_minor) {
        return std::strong_ordering::greater;
    } else if (m_minor < rhs.m_minor) {
        return std::strong_ordering::less;
    }

    if (m_patch > rhs.m_patch) {
        return std::strong_ordering::greater;
    } else if (m_patch < rhs.m_patch) {
        return std::strong_ordering::less;
    }

    PartIterator lhs_it, rhs_it;

    for (lhs_it = pre_release_begin(), rhs_it = rhs.pre_release_begin();
         (lhs_it < pre_release_end()) || (rhs_it < rhs.pre_release_end());
         ++lhs_it, ++rhs_it)
    {
        auto [lhs_part, lhs_len] = *lhs_it;
        auto [rhs_part, rhs_len] = *rhs_it;

        i32 part_cmp
            = std::strncmp(lhs_part, rhs_part, lhs_len < rhs_len ? lhs_len : rhs_len);

        if (part_cmp > 0) {
            return std::strong_ordering::greater;
        } else if (part_cmp < 0) {
            return std::strong_ordering::less;
        }
    }

    for (lhs_it = build_begin(), rhs_it = rhs.build_begin();
         (lhs_it < build_end()) || (rhs_it < rhs.build_end());
         ++lhs_it, ++rhs_it)
    {
        auto [lhs_part, lhs_len] = *lhs_it;
        auto [rhs_part, rhs_len] = *rhs_it;

        i32 part_cmp
            = std::strncmp(lhs_part, rhs_part, lhs_len < rhs_len ? lhs_len : rhs_len);

        if (part_cmp > 0) {
            return std::strong_ordering::greater;
        } else if (part_cmp < 0) {
            return std::strong_ordering::less;
        }
    }

    return std::strong_ordering::equal;
}

hemlock::SemanticVersion::PartIterator::PartIterator() :
    m_target(nullptr), m_offset(0), m_length(0) {
    // Empty.
}

hemlock::SemanticVersion::PartIterator::PartIterator(
    const SemanticVersion& semver,
    PartIteratorTarget     target,
    size_t                 offset,
    size_t                 length
) :
    m_offset(offset), m_length(length) {
    if (target == PartIteratorTarget::PRE_RELEASE) {
        m_target = semver.m_pre_release;
    } else {
        m_target = semver.m_build;
    }

    m_target_length = std::strlen(m_target);
}

std::partial_ordering
hemlock::SemanticVersion::PartIterator::operator<=>(const PartIterator& rhs) const {
    if (m_target != rhs.m_target) return std::partial_ordering::unordered;

    if (m_offset < rhs.m_offset) {
        return std::partial_ordering::less;
    } else if (m_offset > rhs.m_offset) {
        return std::partial_ordering::greater;
    }

    return std::partial_ordering::equivalent;
}

hemlock::SemanticVersion::PartIterator&
hemlock::SemanticVersion::PartIterator::operator++() {
#if DEBUG
    assert(m_offset + m_length <= m_target_length);
#endif

    auto it = std::find(
        m_target + m_offset + m_length + 1, m_target + m_target_length, '.'
    );
    auto new_part_end = it - m_target;

    m_offset += m_length + 1;
    m_length = new_part_end - m_offset;

    return *this;
}

hemlock::SemanticVersion::PartIterator
hemlock::SemanticVersion::PartIterator::operator++(int) {
#if DEBUG
    assert(m_offset + m_length <= m_target_length);
#endif

    auto it = std::find(
        m_target + m_offset + m_length + 1, m_target + m_target_length, '.'
    );
    auto new_part_end = it - m_target;

    PartIterator tmp{};

    tmp.m_target        = m_target;
    tmp.m_target_length = m_target_length;

    tmp.m_offset = m_offset + m_length + 1;
    tmp.m_length = new_part_end - tmp.m_offset;

    return tmp;
}

hemlock::SemanticVersion::PartIterator&
hemlock::SemanticVersion::PartIterator::operator--() {
#if DEBUG
    assert(m_offset >= 0);
#endif

    if (m_offset == 0) {
        m_offset = -1;
        return *this;
    }

    auto it = std::find(
        std::reverse_iterator<const char*>(m_target + m_offset - 1),
        std::reverse_iterator<const char*>(m_target),
        '.'
    );
    auto new_part_begin = (&*it) - m_target;

    if (new_part_begin < 0) {
        m_length = m_offset - 1;
        m_offset = 0;
    } else {
        m_length = m_offset - new_part_begin - 2;
        m_offset = new_part_begin + 1;
    }

    return *this;
}

hemlock::SemanticVersion::PartIterator
hemlock::SemanticVersion::PartIterator::operator--(int) {
#if DEBUG
    assert(m_offset >= 0);
#endif

    if (m_offset == 0) {
        PartIterator tmp{};

        tmp.m_target        = m_target;
        tmp.m_target_length = m_target_length;
        tmp.m_offset        = -1;

        return tmp;
    }

    auto it = std::find(
        std::reverse_iterator<const char*>(m_target + m_offset - 1),
        std::reverse_iterator<const char*>(m_target),
        '.'
    );
    auto new_part_begin = (&*it) - m_target;

    PartIterator tmp{};

    tmp.m_target        = m_target;
    tmp.m_target_length = m_target_length;

    if (new_part_begin < 0) {
        tmp.m_length = m_offset - 1;
        tmp.m_offset = 0;
    } else {
        tmp.m_length = tmp.m_offset - new_part_begin - 2;
        tmp.m_offset = new_part_begin + 1;
    }

    return tmp;
}

std::tuple<const char*, size_t>
hemlock::SemanticVersion::PartIterator::operator*() const {
    return { m_target + m_offset, m_length };
}

bool hemlock::SemanticVersion::load(const std::string& str) {
    reset();

    return parse_c_str(str.c_str());
}

bool hemlock::SemanticVersion::load(const char* c_str) {
    reset();

    return parse_c_str(c_str);
}

void hemlock::SemanticVersion::reset() {
    if (m_pre_release) delete[] m_pre_release;
    if (m_build) delete[] m_build;

    *this = {};
}

hemlock::SemanticVersion::PartIterator
hemlock::SemanticVersion::pre_release_begin() const {
    if (!m_pre_release)
        return PartIterator(*this, PartIteratorTarget::PRE_RELEASE, 0, 0);

    auto it = std::find(m_pre_release, m_pre_release + std::strlen(m_pre_release), '.');
    size_t first_part_len = it - m_pre_release;

    return PartIterator(*this, PartIteratorTarget::PRE_RELEASE, 0, first_part_len);
}

hemlock::SemanticVersion::PartIterator
hemlock::SemanticVersion::pre_release_end() const {
    if (!m_pre_release)
        return PartIterator(*this, PartIteratorTarget::PRE_RELEASE, 0, 0);

    size_t pre_release_len = std::strlen(m_pre_release);

    return PartIterator(*this, PartIteratorTarget::PRE_RELEASE, pre_release_len + 1, 0);
}

hemlock::SemanticVersion::PartIterator hemlock::SemanticVersion::build_begin() const {
    if (!m_build) return PartIterator(*this, PartIteratorTarget::BUILD, 0, 0);

    auto   it             = std::find(m_build, m_build + std::strlen(m_build), '.');
    size_t first_part_len = it - m_build;

    return PartIterator(*this, PartIteratorTarget::BUILD, 0, first_part_len);
}

hemlock::SemanticVersion::PartIterator hemlock::SemanticVersion::build_end() const {
    if (!m_build) return PartIterator(*this, PartIteratorTarget::BUILD, 0, 0);

    size_t build_len = std::strlen(m_build);

    return PartIterator(*this, PartIteratorTarget::BUILD, build_len + 1, 0);
}

const char* hemlock::SemanticVersion::c_str() const {
    // Convert major, minor, and patch to their string representation.
    std::array<char, 10> major, minor, patch;

    auto [major_end, major_ec] = std::to_chars(major.begin(), major.end(), m_major);
    auto [minor_end, minor_ec] = std::to_chars(minor.begin(), minor.end(), m_minor);
    auto [patch_end, patch_ec] = std::to_chars(patch.begin(), patch.end(), m_patch);

    // Calculate length of major, minor, and patch strings.
    size_t major_length = major_end - major.data();
    size_t minor_length = minor_end - minor.data();
    size_t patch_length = patch_end - patch.data();

    // Calculate the length of the final version string.
    size_t version_str_length = major_length + minor_length + patch_length + 2;

    size_t pre_release_length = 0;
    if (m_pre_release) {
        ++version_str_length;

        pre_release_length = std::strlen(m_pre_release);
        version_str_length += pre_release_length;
    }

    size_t build_length = 0;
    if (m_build) {
        ++version_str_length;

        build_length       = std::strlen(m_build);
        version_str_length += build_length;
    }

    char* version_str = new char[version_str_length + 1];

    size_t offset = 0;
    // MAJOR
    std::memcpy(version_str, major.data(), major_length);
    offset += major_length;
    // .
    version_str[offset++] = '.';
    // MINOR
    std::memcpy(version_str + offset, minor.data(), minor_length);
    offset += minor_length;
    // .
    version_str[offset++] = '.';
    // PATCH
    std::memcpy(version_str + offset, patch.data(), patch_length);
    offset += patch_length;

    if (m_pre_release) {
        // -
        version_str[offset++] = '-';
        // PRE_RELEASE
        std::memcpy(
            version_str + offset, const_cast<char*>(m_pre_release), pre_release_length
        );
        offset += pre_release_length;
    }

    if (m_build) {
        // +
        version_str[offset++] = '+';
        // BUILD
        std::memcpy(version_str + offset, const_cast<char*>(m_build), build_length);
        offset += build_length;
    }

    version_str[offset] = '\0';

#if DEBUG
    assert(offset == version_str_length);
#endif  // DEBUG

    return version_str;
}

std::string hemlock::SemanticVersion::string() const {
    // TODO(Matthew): this makes a copy...
    return std::string{ c_str() };
}

bool hemlock::SemanticVersion::parse_c_str(const char* c_str) {
    size_t version_str_length = std::strlen(c_str);

    // Get index of MAJOR-MINOR separator.
    auto maj_min_sep_it = std::find(c_str, c_str + version_str_length, '.');
    if (maj_min_sep_it == c_str + version_str_length) {
        return false;
    }
    size_t maj_min_sep_idx = maj_min_sep_it - c_str;

    // Read major.
    auto [major_ptr, major_ec]
        = std::from_chars(c_str, c_str + maj_min_sep_idx, m_major);
    if (major_ptr == c_str) return false;

    // Get index of MINOR-PATCH separator.
    auto min_pat_sep_it
        = std::find(c_str + maj_min_sep_idx + 1, c_str + version_str_length, '.');
    if (min_pat_sep_it == c_str + version_str_length) {
        return false;
    }
    size_t min_pat_sep_idx = min_pat_sep_it - c_str;

    // Read minor.
    auto [minor_ptr, minor_ec] = std::from_chars(
        c_str + maj_min_sep_idx + 1, c_str + min_pat_sep_idx, m_minor
    );
    if (minor_ptr == c_str) return false;

    // Get index of PATCH-PRE_RELEASE separator if it exists.
    auto pre_release_sep_it
        = std::find(c_str + min_pat_sep_idx + 1, c_str + version_str_length, '-');
    size_t pre_release_sep_idx = pre_release_sep_it - c_str;

    if (pre_release_sep_it == c_str + version_str_length) {
        // No pre-release component.

        // Get index of (PATCH|PRE_RELEASE)-BUILD separator if it exists.
        auto build_sep_it
            = std::find(c_str + min_pat_sep_idx + 1, c_str + version_str_length, '+');
        size_t build_sep_idx = build_sep_it - c_str;

        if (build_sep_it == c_str + version_str_length) {
            // No pre-release or build components.

            // Read patch.
            auto [patch_ptr, patch_ec] = std::from_chars(
                c_str + min_pat_sep_idx + 1, c_str + version_str_length, m_patch
            );
            if (patch_ptr == c_str) return false;
        } else {
            // Build component but not pre-release component.

            // Read patch.
            auto [patch_ptr, patch_ec] = std::from_chars(
                c_str + min_pat_sep_idx + 1, c_str + build_sep_idx, m_patch
            );
            if (patch_ptr == c_str) return false;

            // Validate build.
            if (!validate_build(
                    c_str + build_sep_idx + 1, version_str_length - build_sep_idx - 1
                ))
                return false;

            // Read build.
            char* build_str = new char[version_str_length - build_sep_idx];

            std::memcpy(
                build_str,
                const_cast<char*>(c_str) + build_sep_idx + 1,
                version_str_length - build_sep_idx - 1
            );
            build_str[version_str_length - build_sep_idx - 1] = '\0';

            m_build = build_str;
        }
    } else {
        // Get index of (PATCH|PRE_RELEASE)-BUILD separator if it exists.
        auto build_sep_it = std::find(
            c_str + pre_release_sep_idx + 1, c_str + version_str_length, '+'
        );
        size_t build_sep_idx = build_sep_it - c_str;

        if (build_sep_it == c_str + version_str_length) {
            // Pre-release component but no build component.

            // Read patch.
            auto [patch_ptr, patch_ec] = std::from_chars(
                c_str + min_pat_sep_idx + 1, c_str + pre_release_sep_idx, m_patch
            );
            if (patch_ptr == c_str) return false;

            // Validate pre-release.
            if (!validate_pre_release(
                    c_str + pre_release_sep_idx + 1,
                    version_str_length - pre_release_sep_idx - 1
                ))
                return false;

            // Read pre-release.
            char* pre_release_str = new char[version_str_length - pre_release_sep_idx];

            std::memcpy(
                pre_release_str,
                const_cast<char*>(c_str) + pre_release_sep_idx + 1,
                version_str_length - pre_release_sep_idx - 1
            );
            pre_release_str[version_str_length - pre_release_sep_idx - 1] = '\0';

            m_pre_release = pre_release_str;
        } else {
            // Pre-release and build components.

            // Read patch.
            auto [patch_ptr, patch_ec] = std::from_chars(
                c_str + min_pat_sep_idx + 1, c_str + pre_release_sep_idx, m_patch
            );
            if (patch_ptr == c_str) return false;

            // Validate pre-release.
            if (!validate_pre_release(
                    c_str + pre_release_sep_idx + 1,
                    build_sep_idx - pre_release_sep_idx - 1
                ))
                return false;

            // Validate pre-release.
            if (!validate_build(
                    c_str + build_sep_idx + 1, version_str_length - build_sep_idx - 1
                ))
                return false;

            // Read pre-release.
            char* pre_release_str = new char[build_sep_idx - pre_release_sep_idx];

            std::memcpy(
                pre_release_str,
                const_cast<char*>(c_str) + pre_release_sep_idx + 1,
                build_sep_idx - pre_release_sep_idx - 1
            );
            pre_release_str[build_sep_idx - pre_release_sep_idx - 1] = '\0';

            m_pre_release = pre_release_str;

            // Read build.
            char* build_str = new char[version_str_length - build_sep_idx];

            std::memcpy(
                build_str,
                const_cast<char*>(c_str) + build_sep_idx + 1,
                version_str_length - build_sep_idx - 1
            );
            build_str[version_str_length - build_sep_idx - 1] = '\0';

            m_build = build_str;
        }
    }

    return true;
}

bool hemlock::SemanticVersion::validate_pre_release(const char* start, size_t len) {
    bool last_char_was_dot = true;

    // If this is true, then the current part is an alphanumeric identifier, otherwise
    // it could be either an alphanumeric or numeric identifier as per the Backus-Naur
    // Form Grammar of SemVer.
    bool curr_part_is_alphanumeric = false;
    // If this is true, then this must either be alphanumeric in nature or followed by
    // a dot.
    bool curr_part_started_with_zero = false;

    for (size_t offset = 0; offset < len; ++offset) {
        if (last_char_was_dot) {
            last_char_was_dot = false;

            // SemVer requries pre-release parts starting with 0 be alphanumeric or
            // immediately followed by a dot.
            if (start[offset] == '0') {
                // Look ahead for a dot, and if there continue from after that dot.
                if (start[offset + 1] == '.') {
                    ++offset;

                    last_char_was_dot           = true;
                    curr_part_is_alphanumeric   = false;
                    curr_part_started_with_zero = false;
                    continue;
                }

                // No immediate dot, so we'll have to look out for if the part is indeed
                // alphanumeric.
                curr_part_started_with_zero = true;
            } else if (!is_non_digit(start[offset]) && !is_digit(start[offset])) {
                return false;
            }
        } else {
            if (start[offset] == '.') {
                // If we have reached a dot here without finding a part starting with a
                // zero to be alphanumeric, then we have an invalid SemVer string.
                if (curr_part_started_with_zero && !curr_part_is_alphanumeric)
                    return false;

                last_char_was_dot           = true;
                curr_part_is_alphanumeric   = false;
                curr_part_started_with_zero = false;
                continue;
            }

            // If current part started with a zero, and we have located a non-digit
            // character, we have found that the part is indeed alphanumeric.
            if (curr_part_started_with_zero && is_non_digit(start[offset]))
                curr_part_is_alphanumeric = true;

            // Usual check that character is either a valid non-digit or a digit.
            if (!is_non_digit(start[offset]) && !is_digit(start[offset])) return false;
        }
    }

    // If we have reached the end finding the last part to start with a zero but to not
    //  be alphanumeric, then we have an invalid SemVer string.
    if (curr_part_started_with_zero && !curr_part_is_alphanumeric) return false;

    // Cannot end on a dot.
    if (last_char_was_dot) return false;

    return true;
}

bool hemlock::SemanticVersion::validate_build(const char* start, size_t len) {
    bool last_char_was_dot = true;

    for (size_t offset = 0; offset < len; ++offset) {
        if (start[offset] == '.') {
            if (last_char_was_dot) {
                return false;
            } else {
                last_char_was_dot = true;
                continue;
            }
        }

        last_char_was_dot = false;

        if (!is_non_digit(start[offset]) && !is_digit(start[offset])) return false;
    }

    // Cannot end on a dot.
    if (last_char_was_dot) return false;

    return true;
}

bool hemlock::SemanticVersion::is_non_digit(char ch) {
    return is_letter(ch) || ch == '-';
}

bool hemlock::SemanticVersion::is_digit(char ch) {
    return std::isdigit(ch);
}

bool hemlock::SemanticVersion::is_positive_digit(char ch) {
    return std::isdigit(ch) && ch != '0';
}

bool hemlock::SemanticVersion::is_letter(char ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

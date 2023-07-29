#ifndef __hemlock_version_semver_h
#define __hemlock_version_semver_h

namespace hemlock {
    /**
     * @brief Versioning following the semantic versioning 2.0.0 rules
     * (https://semver.org).
     */
    class SemanticVersion {
    public:
        SemanticVersion();
        SemanticVersion(ui32 major, ui32 minor, ui32 patch);
        SemanticVersion(ui32 major, ui32 minor, ui32 patch, const char* pre_release);
        SemanticVersion(
            ui32        major,
            ui32        minor,
            ui32        patch,
            const char* pre_release,
            const char* build
        );
        ~SemanticVersion();

        bool                 operator==(const SemanticVersion& rhs) const;
        std::strong_ordering operator<=>(const SemanticVersion& rhs) const;

        enum class PartIteratorTarget {
            PRE_RELEASE,
            BUILD
        };

        class PartIterator {
            friend class SemanticVersion;
        public:
            PartIterator();
            PartIterator(
                const SemanticVersion& semver,
                PartIteratorTarget     target,
                size_t                 offset,
                size_t                 length
            );

            ~PartIterator() { /* Empty. */
            }

            std::partial_ordering operator<=>(const PartIterator& rhs) const;

            PartIterator& operator++();
            PartIterator  operator++(int);

            PartIterator& operator--();
            PartIterator  operator--(int);

            std::tuple<const char*, size_t> operator*() const;
        protected:
            const char* m_target;
            i64         m_target_length;

            i64 m_offset, m_length;
        };

        bool load(const std::string& str);
        bool load(const char* c_str);

        void reset();

        ui32 major() const { return m_major; }

        ui32 minor() const { return m_minor; }

        ui32 patch() const { return m_patch; }

        const char* pre_release() const { return m_pre_release; }

        PartIterator pre_release_begin() const;
        PartIterator pre_release_end() const;

        const char* build() const { return m_build; }

        PartIterator build_begin() const;
        PartIterator build_end() const;

        const char* c_str() const;
        std::string string() const;
    protected:
        bool parse_c_str(const char* c_str);

        bool validate_pre_release(const char* start, size_t len);
        bool validate_build(const char* start, size_t len);

        inline bool is_non_digit(char ch);
        inline bool is_digit(char ch);
        inline bool is_positive_digit(char ch);
        inline bool is_letter(char ch);

        ui32        m_major, m_minor, m_patch;
        const char *m_pre_release, *m_build;
    };
}  // namespace hemlock

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

#endif  // __hemlock_version_semver_h

#ifndef __hemlock_semver_h
#define __hemlock_semver_h

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
        SemanticVersion(ui32 major, ui32 minor, ui32 patch, const char* build);
        ~SemanticVersion();

        bool load(const std::string& str);
        bool load(const char* c_str);

        void reset();

        ui32 major() const { return m_major; }

        ui32 minor() const { return m_minor; }

        ui32 patch() const { return m_patch; }

        const char* pre_release() const { return m_pre_release; }

        const char* build() const { return m_build; }

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

#endif  // __hemlock_semver_h

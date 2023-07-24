#ifndef __hemlock_mod_metadata_hpp
#define __hemlock_mod_metadata_hpp

namespace hemlock {
    namespace mod {
        // TODO(Matthew): UUID? Can assign a hashable ID at runtime.
        using ModID = ui64;
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

namespace hemlock {
    namespace mod {
        using ModCompatibilities
            = std::unordered_map<hmod::ModID, std::optional<hemlock::Versions>>;
    }
}  // namespace hemlock

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModMetadata,
    (_version, ui16),
    (_reserved, ui16),
    (id, hmod::ModID),
    (name, std::string),
    (authors, std::vector<std::string>),
    (description, std::string),
    (version, hemlock::SemanticVersion),
    (last_updated, std::chrono::sys_seconds),
    (hard_depends, std::optional<hmod::ModCompatibilities>),
    (soft_depends, std::optional<hmod::ModCompatibilities>),
    (hard_wanted_by, std::optional<hmod::ModCompatibilities>),
    (soft_wanted_by, std::optional<hmod::ModCompatibilities>),
    (compatible, std::optional<hmod::ModCompatibilities>),
    (incompatible, std::optional<hmod::ModCompatibilities>)
)

#endif  // __hemlock_mod_metadata_hpp

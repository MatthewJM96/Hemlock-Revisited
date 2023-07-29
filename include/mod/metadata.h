#ifndef __hemlock_mod_metadata_h
#define __hemlock_mod_metadata_h

namespace hemlock {
    namespace mod {
        using ModCompatibilities = std::unordered_map<
            boost::uuids::uuid,
            std::optional<hemlock::Versions>,
            boost::hash<boost::uuids::uuid>>;
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModMetadata,
    (_version, ui16),
    (_reserved, ui16),
    (id, boost::uuids::uuid),
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

#endif  // __hemlock_mod_metadata_h

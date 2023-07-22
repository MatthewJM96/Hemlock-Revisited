#ifndef __hemlock_mod_metadata_hpp
#define __hemlock_mod_metadata_hpp

#include "io/serialisation.hpp"

namespace hemlock {
    namespace mod {
        // TODO(Matthew): UUID? Can assign a hashable ID at runtime.
        using ModID = ui64;
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModMetadata,
    (_metadata_version, ui16),
    (_reserved, ui16),
    (id, hmod::ModID),
    (name, std::string),
    (authors, std::vector<std::string>),
    (description, std::string),
    (version, hemlock::SemanticVersion)
    // Last updated?
    // Version?
)

#endif  // __hemlock_mod_metadata_hpp

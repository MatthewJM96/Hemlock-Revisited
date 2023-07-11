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
    (id, hmod::ModID),
    (name, std::string),
    (authors, std::vector<std::string>),
    (description, std::string)
    // Last updated?
    // Version?
)

#endif  // __hemlock_mod_metadata_hpp

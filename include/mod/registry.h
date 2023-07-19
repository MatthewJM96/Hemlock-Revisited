#ifndef __hemlock_mod_registry_hpp
#define __hemlock_mod_registry_hpp

#include "io/serialisation.hpp"

#include "mod.h"

namespace hemlock {
    namespace mod {
        using ModRegistryEntries = std::unordered_map<hmod::ModID, hio::fs::path>;
    }
}  // namespace hemlock
namespace hmod = hemlock::mod;

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModRegistry,
    (_registry_version, ui16),
    (_reserved, ui16),
    (mods, hmod::ModRegistryEntries)
)

#endif  // __hemlock_mod_registry_hpp

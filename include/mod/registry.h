#ifndef __hemlock_mod_registry_hpp
#define __hemlock_mod_registry_hpp

#include "io/serialisation.hpp"

#include "metadata.h"
#include "mod.h"

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModDirectories,
    (_version, ui16),
    (_reserved, ui16),
    (mod_directories, std::vector<hio::fs::path>)
)

namespace hemlock {
    namespace mod {
        using ModRegistry = UUIDMap<ModMetadata>;
    }
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_registry_hpp

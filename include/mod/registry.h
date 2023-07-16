#ifndef __hemlock_mod_registry_hpp
#define __hemlock_mod_registry_hpp

#include "io/filesystem.hpp"
#include "io/serialisation.hpp"

H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModRegistry,
    (_registry_version, ui16),
    (_reserved, ui16),
    (authors, std::vector<hio::fs::path>),
    // Last updated?
    // Version?
)

#endif  // __hemlock_mod_registry_hpp

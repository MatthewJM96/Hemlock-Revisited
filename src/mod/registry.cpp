#include "stdafx.h"

#include "mod/registry.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModRegistry,
    (_version, ui16),
    (_reserved, ui16),
    (mods, hmod::ModRegistryEntries)
)

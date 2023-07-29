#include "stdafx.h"

#include "mod/registry.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModDirectories,
    (_version, ui16),
    (_reserved, ui16),
    (mod_directories, std::vector<hio::fs::path>)
)

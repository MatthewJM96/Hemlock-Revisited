#include "stdafx.h"

#include "mod/metadata.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModMetadata,
    (_metadata_version, ui16),
    (_reserved, ui16),
    (id, hmod::ModID),
    (name, std::string),
    (authors, std::vector<std::string>),
    (description, std::string)
    // Last updated?
    // Version?
)

#include "stdafx.h"

#include "mod/metadata.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModMetadata,
    (id, hmod::ModID),
    (name, std::string),
    (authors, std::vector<std::string>),
    (description, std::string)
    // Last updated?
    // Version?
)

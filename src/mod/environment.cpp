#include "stdafx.h"

#include "mod/environment.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    LoadOrder,
    (_load_order_version, ui16),
    (_reserved, ui16),
    (id, hmod::LoadOrderID),
    (name, std::string),
    (mods, std::vector<hmod::ModID>),
    (description, std::string)
    // Last updated?
    // Version?
)

#include "stdafx.h"

#include "mod/load_order/state.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    LoadOrder,
    (_version, ui16),
    (_reserved, ui16),
    (id, boost::uuids::uuid),
    (name, std::string),
    (mods, std::vector<boost::uuids::uuid>),
    (description, std::string),
    (version, hemlock::SemanticVersion),
    (last_updated, std::chrono::sys_seconds)
)

H_DEF_ENUM_WITH_SERIALISATION(hemlock::mod, LoadOrderState)

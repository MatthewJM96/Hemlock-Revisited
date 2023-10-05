#include "stdafx.h"

#include "mod/state.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    ModMetadata,
    (id, boost::uuids::uuid),
    (name, std::string),
    (authors, std::vector<std::string>),
    (description, std::string),
    (version, hemlock::SemanticVersion),
    (last_updated, std::chrono::sys_seconds),
    (hard_depends, std::optional<hmod::ModCompatibilities>),
    (soft_depends, std::optional<hmod::ModCompatibilities>),
    (hard_wanted_by, std::optional<hmod::ModCompatibilities>),
    (soft_wanted_by, std::optional<hmod::ModCompatibilities>),
    (compatible, std::optional<hmod::ModCompatibilities>),
    (incompatible, std::optional<hmod::ModCompatibilities>)
)

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    LoadOrder,
    (id, boost::uuids::uuid),
    (name, std::string),
    (mods, std::vector<boost::uuids::uuid>),
    (description, std::string),
    (version, hemlock::SemanticVersion),
    (last_updated, std::chrono::sys_seconds)
)

H_DEF_ENUM_WITH_SERIALISATION(hemlock::mod, LoadOrderState)

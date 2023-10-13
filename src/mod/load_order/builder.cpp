#include "stdafx.h"

#include "mod/load_order/builder.h"

#include "mod/load_order/validate/compatibility.h"
#include "mod/load_order/validate/consistency.h"

void hmod::LoadOrderBuilder::init(const ModManager* mod_manager) {
    m_mod_manager = mod_manager;

#ifdef DEBUG
    assert(mod_manager != nullptr);
#endif
}

hmod::LoadOrderState hmod::LoadOrderBuilder::set_load_order(const LoadOrder& load_order
) { }

void hmod::LoadOrderBuilder::dispose() {
    m_mod_manager = nullptr;

    m_load_order = {};
}

void hmod::LoadOrderBuilder::set_name(std::string&& name) {
    m_load_order.name = std::forward<std::string>(name);
}

void hmod::LoadOrderBuilder::set_description(std::string&& description) {
    m_load_order.description = std::forward<std::string>(description);
}

void hmod::LoadOrderBuilder::set_version(hemlock::SemanticVersion&& version) {
    m_load_order.version = std::forward<hemlock::SemanticVersion>(version);
}

hmod::LoadOrderState hmod::LoadOrderBuilder::add_mod(
    const hemlock::UUID& id,
    bool                 allow_version_mismatch /*= false*/,
    bool                 allow_invaid_order = /*true*/
) {
    const ModMetadata* mod_metadata = m_mod_manager->get_mod(id);

    if (!mod_metadata) return LoadOrderState::MISSING_METADATA;

    if (!is_self_consistent(*mod_metadata)) return LoadOrderState::INCONSISTENT;

    LoadOrderState compatiblity
        = is_compatible(*mod_metadata, m_load_order, m_mod_manager->get_mods());
    if (compatiblity == LoadOrderState::INCOMPATIBLE) {
        return LoadOrderState::INCOMPATIBLE;
    } else if (!allow_version_mismatch && compatiblity == LoadOrderState::VERSION_MISMATCH)
    {
        return LoadOrderState::VERSION_MISMATCH;
    }

    m_load_order.mods.emplace_back(id);

    // TODO(Matthew): progressively build up dependency graph? if we do this we can run
    //                a topological sort each time to report introduction of a cycle.
}

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
) {
    (void)load_order;

    return hmod::LoadOrderState::SENTINEL;
}

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

std::pair<bool, hmod::LoadOrderState> hmod::LoadOrderBuilder::add_mod(
    const hemlock::UUID& id,
    bool                 allow_version_mismatch /*= false*/,
    bool                 allow_invalid_order /*= true*/
) {
    // TODO(Matthew): remove me
    (void)allow_invalid_order;

    auto it = std::find(m_load_order.mods.begin(), m_load_order.mods.end(), id);
    if (it != m_load_order.mods.end()) return { false, LoadOrderState::VALID };

    const ModMetadata* mod_metadata = m_mod_manager->get_mod(id);

    if (!mod_metadata) return { false, LoadOrderState::MISSING_METADATA };

    if (!is_self_consistent(*mod_metadata))
        return { false, LoadOrderState::INCONSISTENT };

    LoadOrderState compatiblity
        = is_compatible(*mod_metadata, m_load_order, m_mod_manager->get_mods());
    if (compatiblity == LoadOrderState::INCOMPATIBLE) {
        return { false, LoadOrderState::INCOMPATIBLE };
    } else if (!allow_version_mismatch && compatiblity == LoadOrderState::VERSION_MISMATCH)
    {
        return { false, LoadOrderState::VERSION_MISMATCH };
    }

    m_load_order.mods.emplace_back(id);

    m_mod_vertex_map[id]                   = boost::add_vertex(m_dependency_graph);
    m_vertex_mod_map[m_mod_vertex_map[id]] = id;

    if (mod_metadata->hard_depends.has_value()) {
        for (const auto& [depends_id, _] : mod_metadata->hard_depends.value()) {
            if (m_mod_vertex_map.contains(depends_id)) {
                auto [edge, _2] = boost::add_edge(
                    m_mod_vertex_map[depends_id],
                    m_mod_vertex_map[id],
                    m_dependency_graph
                );

                (void)edge;
                (void)_2;
            }
        }
    }

    if (mod_metadata->soft_depends.has_value()) {
        for (const auto& [depends_id, _] : mod_metadata->soft_depends.value()) {
            if (m_mod_vertex_map.contains(depends_id)) {
                auto [edge, _2] = boost::add_edge(
                    m_mod_vertex_map[depends_id],
                    m_mod_vertex_map[id],
                    m_dependency_graph
                );

                (void)edge;
                (void)_2;
            }
        }
    }

    if (mod_metadata->hard_wanted_by.has_value()) {
        for (const auto& [wanted_by_id, _] : mod_metadata->hard_wanted_by.value()) {
            if (m_mod_vertex_map.contains(wanted_by_id)) {
                auto [edge, _2] = boost::add_edge(
                    m_mod_vertex_map[id],
                    m_mod_vertex_map[wanted_by_id],
                    m_dependency_graph
                );

                (void)edge;
                (void)_2;
            }
        }
    }

    if (mod_metadata->soft_wanted_by.has_value()) {
        for (const auto& [wanted_by_id, _] : mod_metadata->soft_wanted_by.value()) {
            if (m_mod_vertex_map.contains(wanted_by_id)) {
                auto [edge, _2] = boost::add_edge(
                    m_mod_vertex_map[id],
                    m_mod_vertex_map[wanted_by_id],
                    m_dependency_graph
                );

                (void)edge;
                (void)_2;
            }
        }
    }

    // TODO(Matthew): complete the logic...
    return { false, LoadOrderState::SENTINEL };
}

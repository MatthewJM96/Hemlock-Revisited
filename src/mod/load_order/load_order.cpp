#include "stdafx.h"

#include "mod/load_order/load_order.h"
#include "mod/load_order/validate/compatibility.h"
#include "mod/load_order/validate/consistency.h"
#include "mod/load_order/validate/order.h"
#include "mod/metadata.h"
#include "mod/registry.h"

hmod::LoadOrderState
hmod::validate_load_order(const LoadOrder& load_order, const ModRegistry& registry) {
    // Not efficient but first do a pass to check if we're missing any metadata needed
    // to validate the load order.

    try {
        for (const auto& id : load_order.mods) {
            registry.at(id);
        }
    } catch (std::out_of_range&) {
        return LoadOrderState::MISSING_METADATA;
    }

    // We now know all metadata we need does indeed exist, so we will iterate each mod
    // in the load order now, and check its conditions.

    for (size_t curr_idx = 0; curr_idx < load_order.mods.size(); ++curr_idx) {
        const auto& curr_id = load_order.mods[curr_idx];

        const ModMetadata& curr_metadata = registry.at(curr_id);

        if (!is_self_consistent(curr_metadata)) {
            return LoadOrderState::INVALID_ORDER;
        }

        if (!validate_depends(curr_idx, load_order, registry)) {
            return LoadOrderState::INVALID_ORDER;
        }

        if (!validate_wanted_by(curr_idx, load_order, registry)) {
            return LoadOrderState::INVALID_ORDER;
        }

        auto comp_state = is_compatible(curr_idx, load_order, registry);
        if (comp_state != LoadOrderState::COMPATIBLE) return comp_state;
    }

    return LoadOrderState::VALID;
}

hmod::LoadOrderState
hmod::make_load_order_valid(LoadOrder& load_order, const ModRegistry& registry) {
    // Not efficient but first do a pass to check if we're missing any metadata needed
    // to validate the load order.

    try {
        for (const auto& id : load_order.mods) {
            registry.at(id);
        }
    } catch (std::out_of_range&) {
        return LoadOrderState::MISSING_METADATA;
    }

    // No missing metadata, we can go ahead and build a graph to do a topological sort
    // over.

    using LoadOrderGraph = boost::adjacency_list<
        boost::vecS,
        boost::vecS,
        boost::directedS,
        boost::property<boost::vertex_color_t, boost::default_color_type>,
        boost::property<boost::edge_weight_t, bool>>;
    // using LoadOrderEdge   = boost::graph_traits<LoadOrderGraph>::edge_descriptor;
    using LoadOrderVertex = boost::graph_traits<LoadOrderGraph>::vertex_descriptor;

    LoadOrderGraph g(load_order.mods.size());

    std::unordered_map<UUID, LoadOrderVertex> mod_vertex_map;

    std::unordered_map<LoadOrderVertex, hemlock::UUID> vertex_mod_map;

    // Tells whether the edge represents a hard depends/wanted-by constraint, or
    // a soft one. If we encounter a cycle involving a hard dependency we cannot
    using EdgeHardnessMap =
        typename boost::property_map<LoadOrderGraph, boost::edge_weight_t>::type;
    EdgeHardnessMap edge_hardness_map;

    for (const auto& id : load_order.mods) {
        if (!mod_vertex_map.contains(id)) {
            mod_vertex_map[id]                 = boost::add_vertex(g);
            vertex_mod_map[mod_vertex_map[id]] = id;
        }

        const auto& metadata = registry.at(id);

        if (metadata.hard_depends.has_value()) {
            for (const auto& [depends_id, _] : metadata.hard_depends.value()) {
                if (!mod_vertex_map.contains(depends_id)) {
                    mod_vertex_map[depends_id]                 = boost::add_vertex(g);
                    vertex_mod_map[mod_vertex_map[depends_id]] = depends_id;
                }

                auto [edge, _2] = boost::add_edge(
                    mod_vertex_map[depends_id], mod_vertex_map[id], g
                );
                edge_hardness_map[edge] = true;
            }
        }

        if (metadata.hard_wanted_by.has_value()) {
            for (const auto& [wanted_by_id, _] : metadata.hard_wanted_by.value()) {
                if (!mod_vertex_map.contains(wanted_by_id)) {
                    mod_vertex_map[wanted_by_id]                 = boost::add_vertex(g);
                    vertex_mod_map[mod_vertex_map[wanted_by_id]] = wanted_by_id;
                }

                auto [edge, _2] = boost::add_edge(
                    mod_vertex_map[id], mod_vertex_map[wanted_by_id], g
                );
                edge_hardness_map[edge] = true;
            }
        }
    }

    std::vector<LoadOrderVertex> sorted_on_hard_only;

    try {
        boost::topological_sort(g, std::back_inserter(sorted_on_hard_only));
    } catch (boost::not_a_dag&) {
        std::cout
            << "Encountered some loop preventing satisfaction of hard relationships."
            << std::endl;
    }

    for (const auto& id : load_order.mods) {
        if (!mod_vertex_map.contains(id)) {
            mod_vertex_map[id]                 = boost::add_vertex(g);
            vertex_mod_map[mod_vertex_map[id]] = id;
        }

        const auto& metadata = registry.at(id);

        if (metadata.soft_depends.has_value()) {
            for (const auto& [depends_id, _] : metadata.soft_depends.value()) {
                if (!mod_vertex_map.contains(depends_id)) {
                    mod_vertex_map[depends_id]                 = boost::add_vertex(g);
                    vertex_mod_map[mod_vertex_map[depends_id]] = depends_id;
                }

                auto [edge, _2] = boost::add_edge(
                    mod_vertex_map[depends_id], mod_vertex_map[id], g
                );
                edge_hardness_map[edge] = false;
            }
        }

        if (metadata.soft_wanted_by.has_value()) {
            for (const auto& [wanted_by_id, _] : metadata.soft_wanted_by.value()) {
                if (!mod_vertex_map.contains(wanted_by_id)) {
                    mod_vertex_map[wanted_by_id]                 = boost::add_vertex(g);
                    vertex_mod_map[mod_vertex_map[wanted_by_id]] = wanted_by_id;
                }

                auto [edge, _2] = boost::add_edge(
                    mod_vertex_map[id], mod_vertex_map[wanted_by_id], g
                );
                edge_hardness_map[edge] = false;
            }
        }
    }

    std::vector<LoadOrderVertex> sorted_on_both;

    try {
        boost::topological_sort(g, std::back_inserter(sorted_on_both));
    } catch (boost::not_a_dag&) {
        std::cout
            << "Encountered some loop preventing satisfaction of soft relationships."
            << std::endl;
    }

    // Maybe we can specifically locate the problem when cycles appear.
    //  unsure about this as a back edge may not form a loop if it points to the
    //  end of an opposing direction path, detecting this may be hard?

    return LoadOrderState::VALID;
}

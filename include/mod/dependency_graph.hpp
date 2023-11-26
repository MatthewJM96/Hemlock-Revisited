#ifndef __hemlock_mod_load_order_dependency_graph_hpp
#define __hemlock_mod_load_order_dependency_graph_hpp

#include "mod/manager.h"
#include "mod/state.h"

namespace hemlock {
    namespace mod {
        using ModDependencyGraph = boost::adjacency_list<
            boost::vecS,
            boost::vecS,
            boost::directedS,
            boost::property<boost::vertex_color_t, boost::default_color_type>,
            boost::property<boost::edge_weight_t, bool>>;

        using ModDependencyGraphEdge
            = boost::graph_traits<ModDependencyGraph>::edge_descriptor;
        using ModDependencyGraphVertex
            = boost::graph_traits<ModDependencyGraph>::vertex_descriptor;
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_load_order_dependency_graph_hpp

#ifndef __hemlock_algorithm_acs_graph_state_hpp
#define __hemlock_algorithm_acs_graph_state_hpp

namespace hemlock {
    namespace algorithm {
        enum vertex_data_t {
            vertex_data
        };
    }  // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

namespace boost {
    template <>
    struct property_kind<halgo::vertex_data_t> {
        typedef vertex_property_tag type;
    };
}  // namespace boost

namespace hemlock {
    namespace algorithm {
        namespace details {
            template <typename Node>
            using VertexProperties = boost::property<vertex_data_t, Node>;
            template <bool IsWeighted>
            using EdgeProperties = std::conditional_t<
                IsWeighted,
                boost::property<boost::edge_weight_t, f32>,
                boost::no_property>;
        }  // namespace details

        template <typename Node, bool IsWeighted>
        using Graph = boost::adjacency_list<
            boost::setS,
            boost::vecS,
            boost::directedS,
            details::VertexProperties<Node>,
            details::EdgeProperties<IsWeighted>>;

        template <typename Node, bool IsWeighted>
        using VertexDescriptor =
            typename boost::graph_traits<Graph<Node, IsWeighted>>::vertex_descriptor;
        template <typename Node, bool IsWeighted>
        using EdgeDescriptor =
            typename boost::graph_traits<Graph<Node, IsWeighted>>::edge_descriptor;

        template <typename Node, bool IsWeighted>
        using VertexCoordMap =
            typename boost::property_map<Graph<Node, IsWeighted>, vertex_data_t>::type;
        template <typename Node, bool IsWeighted>
        using CoordVertexMap
            = std::unordered_map<Node, VertexDescriptor<Node, IsWeighted>>;
        template <typename Node>
        using PheromoneMap = std::unordered_map<Node, std::unordered_map<Node, f32>>;
        template <typename Node, bool IsWeighted>
        using EdgeWeightMap = typename boost::
            property_map<Graph<Node, IsWeighted>, boost::edge_weight_t>::type;
        template <typename Node, bool IsWeighted>
        using MaybeEdgeWeightMap = std::
            conditional_t<IsWeighted, EdgeWeightMap<Node, IsWeighted>, std::monostate>;

        template <typename Node, bool IsWeighted, typename = void>
        struct _GraphMapState;

        template <typename Node, bool IsWeighted>
        struct _GraphMapState<Node, IsWeighted, typename std::enable_if_t<IsWeighted>> {
            _GraphMapState() {
                vertex_coord_map = boost::get(vertex_data, graph);
                edge_weight_map  = boost::get(boost::edge_weight, graph);
            }

            Graph<Node, IsWeighted>          graph;
            EdgeWeightMap<Node, IsWeighted>  edge_weight_map;
            VertexCoordMap<Node, IsWeighted> vertex_coord_map;
            CoordVertexMap<Node, IsWeighted> coord_vertex_map;
        };

        template <typename Node, bool IsWeighted>
        struct _GraphMapState<
            Node,
            IsWeighted,
            typename std::enable_if_t<!IsWeighted>> {
            _GraphMapState() { vertex_coord_map = boost::get(vertex_data, graph); }

            Graph<Node, IsWeighted>          graph;
            VertexCoordMap<Node, IsWeighted> vertex_coord_map;
            CoordVertexMap<Node, IsWeighted> coord_vertex_map;
        };

        template <typename Node, bool IsWeighted>
        struct GraphMap : public _GraphMapState<Node, IsWeighted> {
            // Empty.
        };
    }  // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_graph_state_hpp

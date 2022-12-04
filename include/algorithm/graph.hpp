#ifndef __hemlock_algorithm_graph_hpp
#define __hemlock_algorithm_graph_hpp

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
            template <typename VertexData>
            using VertexProperties = boost::property<vertex_data_t, VertexData>;
            using EdgeProperties   = boost::property<boost::edge_weight_t, f32>;
        }  // namespace details

        template <typename VertexData>
        using Graph = boost::adjacency_list<
            boost::vecS,
            boost::vecS,
            boost::bidirectionalS,
            details::VertexProperties<VertexData>,
            details::EdgeProperties>;

        template <typename VertexData>
        using VertexDescriptor
            = boost::graph_traits<Graph<VertexData>>::vertex_descriptor;
        template <typename VertexData>
        using EdgeDescriptor
            = boost::graph_traits<Graph<VertexData>>::edge_descriptor;

        template <typename VertexData>
        using EdgeWeightMap
            = boost::property_map<Graph<VertexData>, boost::edge_weight_t>::type;
        template <typename VertexData>
        using VertexCoordMap
            = boost::property_map<Graph<VertexData>, vertex_data_t>::type;
        template <typename VertexData>
        using CoordVertexMap
            = std::unordered_map<VertexData, VertexDescriptor<VertexData>>;

        template <typename VertexData>
        struct GraphMap {
            GraphMap() {
                edge_weight_map  = boost::get(boost::edge_weight, graph);
                vertex_coord_map = boost::get(vertex_data, graph);
            }

            Graph<VertexData>          graph;
            EdgeWeightMap<VertexData>  edge_weight_map;
            VertexCoordMap<VertexData> vertex_coord_map;
            CoordVertexMap<VertexData> coord_vertex_map;
        };
    }  // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_graph_hpp
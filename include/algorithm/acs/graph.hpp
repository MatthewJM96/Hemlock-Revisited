#ifndef __hemlock_algorithm_acs_graph_hpp
#define __hemlock_algorithm_acs_graph_hpp

namespace hemlock {
    namespace algorithm {
        enum vertex_coords_t {
            vertex_coords
        };
        enum edge_in_path_t {
            edge_in_path
        };
    }
}
namespace halgo = hemlock::algorithm;

namespace boost {
    template <> struct property_kind< halgo::vertex_coords_t > { typedef vertex_property_tag type; };
    template <> struct property_kind< halgo::edge_in_path_t  > { typedef edge_property_tag   type; };
}

namespace hemlock {
    namespace algorithm {
        namespace details {
            template <typename VertexCoordType>
            using VertexProperties = boost::property<vertex_coords_t, VertexCoordType>;
            using EdgeProperties   = boost::property<boost::edge_weight_t, float, boost::property<edge_in_path_t, bool>>;
        }

        template <typename VertexCoordType>
        using ACSGraph =
            boost::adjacency_list<
                boost::vecS, boost::vecS,
                boost::bidirectionalS,
                details::VertexProperties<VertexCoordType>,
                details::EdgeProperties
            >;

        namespace details {
            template <typename VertexCoordType>
            using VertexDescriptor = boost::graph_traits<ACSGraph<VertexCoordType>>::vertex_descriptor;
            template <typename VertexCoordType>
            using EdgeDescriptor   = boost::graph_traits<ACSGraph<VertexCoordType>>::edge_descriptor;
        }

        template <typename VertexCoordType>
        using EdgeWeightMap  = boost::property_map<ACSGraph<VertexCoordType>, boost::edge_weight_t>::type;
        template <typename VertexCoordType>
        using EdgeInPathMap  = boost::property_map<ACSGraph<VertexCoordType>, edge_in_path_t>::type;
        template <typename VertexCoordType>
        using VertexCoordMap = boost::property_map<ACSGraph<VertexCoordType>, vertex_coords_t>::type;
        template <typename VertexCoordType>
        using CoordVertexMap = std::unordered_map<VertexCoordType, details::VertexDescriptor<VertexCoordType>>;

        template <typename VertexCoordType>
        struct ACSGraphMap {
            ACSGraph<VertexCoordType>           graph;
            EdgeWeightMap<VertexCoordType>      edge_weight_map;
            EdgeInPathMap<VertexCoordType>      edge_in_path_map;
            VertexCoordMap<VertexCoordType>     vertex_coord_map;
            CoordVertexMap<VertexCoordType>     coord_vertex_map;
        };

        template <typename VertexCoordType>
        void reset_acs_graph_map_best_path(ACSGraphMap<VertexCoordType> map);

        template <typename VertexCoordType>
        void reset_acs_graph_map(ACSGraphMap<VertexCoordType> map);
    }
}
namespace halgo = hemlock::algorithm;

#include "algorithm/acs/graph.inl"

#endif // __hemlock_algorithm_acs_graph_hpp

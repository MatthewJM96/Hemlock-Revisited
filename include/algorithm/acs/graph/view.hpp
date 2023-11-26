#ifndef __hemlock_algorithm_acs_graph_view_hpp
#define __hemlock_algorithm_acs_graph_view_hpp

#include "state.hpp"

namespace hemlock {
    namespace algorithm {
        template <typename Node, bool IsWeighted>
        class GraphMapView {
            using _VertexDescriptor = VertexDescriptor<Node, IsWeighted>;
            using _EdgeDescriptor   = EdgeDescriptor<Node, IsWeighted>;
        public:
            virtual std::tuple<_VertexDescriptor, GraphMap<Node, IsWeighted>*>
            vertex(Node node) = 0;
        };

        template <typename Node, bool IsWeighted>
        class DummyGraphMapView : public GraphMapView<Node, IsWeighted> {
            using _GraphMap         = GraphMap<Node, IsWeighted>;
            using _VertexDescriptor = VertexDescriptor<Node, IsWeighted>;
            using _EdgeDescriptor   = EdgeDescriptor<Node, IsWeighted>;
        public:
            void init(_GraphMap& map) { m_map = &map; }

            std::tuple<_VertexDescriptor, GraphMap<Node, IsWeighted>*> vertex(Node node
            ) override final {
                return { m_map->coord_vertex_map[node], m_map };
            }
        protected:
            _GraphMap* m_map;
        };
    }  // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_graph_view_hpp

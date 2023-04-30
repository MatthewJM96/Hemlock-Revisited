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
    }  // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_graph_view_hpp

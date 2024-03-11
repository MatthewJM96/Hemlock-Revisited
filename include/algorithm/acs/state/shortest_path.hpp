#ifndef __hemlock_algorithm_acs_state_shortest_path_hpp
#define __hemlock_algorithm_acs_state_shortest_path_hpp

namespace hemlock {
    namespace algorithm {
        namespace acs {
            template <typename VertexType, size_t AntCount>
            struct ShortestPath {
                // TODO(Matthew): do we want to just track the destination vertex; this
                //                is sufficient information.
                std::array<std::pair<VertexType, VertexType>, AntCount> steps = {};
                size_t length = std::numeric_limits<size_t>::max();
                bool   found  = false;
            };

            template <typename VertexType>
            struct ShortestPath<VertexType, std::dynamic_extent> {
                std::pair<VertexType, VertexType>, AntCount > * steps = nullptr;
                size_t length = std::numeric_limits<size_t>::max();
                bool   found  = false;
            };
        }  // namespace acs
    }      // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_state_shortest_path_hpp

#ifndef __hemlock_algorithm_acs_state_ant_hpp
#define __hemlock_algorithm_acs_state_ant_hpp

namespace hemlock {
    namespace algorithm {
        namespace acs {
            template <typename VertexType>
            struct Ant {
                bool   found_food   = false;
                bool   did_backstep = false;
                size_t steps_taken  = 0;
                size_t group        = 0;

                std::pair<VertexType, VertexType>* previous_vertices = nullptr;
                std::pair<VertexType, VertexType>  current_vertex    = {};
            };

            template <typename VertexType, size_t AntCount>
            struct AntGroup {
                std::array<Ant<VertexType>*, AntCount> ants = {};
                size_t                                 size = 0;
            };

            template <typename VertexType>
            struct AntGroup<VertexType, std::dynamic_extent> {
                Ant<VertexType>** ants = nullptr;
                size_t            size = 0;
            };
        }  // namespace acs
    }      // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_state_ant_hpp

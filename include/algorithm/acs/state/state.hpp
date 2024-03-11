#ifndef __hemlock_algorithm_acs_state_state_hpp
#define __hemlock_algorithm_acs_state_state_hpp

#include "common.hpp"
#include "pheromone.hpp"

#include "dynamic_exploitation.hpp"
#include "mean_filtering.hpp"

#include "ant.hpp"
#include "shortest_path.hpp"

namespace hemlock {
    namespace algorithm {
        namespace acs {
            struct VanillaState : public CommonStatePart, PhermoneStatePart { };

            struct DynamicExploitationState :
                public VanillaState,
                DynamicExploitationStatePart { };

            struct MeanFilteringState : public VanillaState, MeanFilteringStatePart { };

            struct AllState :
                public VanillaState,
                DynamicExploitationStatePart,
                ACS_MeanFilteringStatePart { };
        }  // namespace acs
    }      // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_state_state_hpp

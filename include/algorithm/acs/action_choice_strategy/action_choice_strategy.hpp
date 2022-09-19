#ifndef __hemlock_algorithm_acs_action_choice_strategy_action_choice_strategy_hpp
#define __hemlock_algorithm_acs_action_choice_strategy_action_choice_strategy_hpp

#include "algorithm/acs/acs.hpp"

namespace hemlock {
    namespace algorithm {
        template <typename VertexDescriptor>
        struct Ant;

        template <typename ActionType>
        class ActionChoiceStrategyBase {
        public:
            template <typename NextActionFinder>
            std::pair<bool, VertexDescriptor<ActionType>> choose(    Ant<VertexDescriptor<ActionType>>& ant,
                                                                                                    f32 exploitation_factor,
                                                                           VertexDescriptor<ActionType> current_vertex,
                                                                            const GraphMap<ActionType>& map      ) {
                return do_choose<NextActionFinder>(ant, exploitation_factor, current_vertex, map);
            }
        };

        // TODO(Matthew): alternative strategies like distance reduction preference (powers of).
    }
}
namespace halgo = hemlock::algorithm;

#endif // __hemlock_algorithm_acs_action_choice_strategy_action_choice_strategy_hpp

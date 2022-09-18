#ifndef __hemlock_algorithm_acs_action_choice_strategy_action_choice_strategy_hpp
#define __hemlock_algorithm_acs_action_choice_strategy_action_choice_strategy_hpp

#include "algorithm/acs.hpp"

namespace hemlock {
    namespace algorithm {
        template <typename ActionType>
        class ActionChoiceStrategyBase {
        public:
            template <typename NextActionFinder>
            VertexDescriptor<ActionType> choose(VertexDescriptor<ActionType> current_vertex, const GraphMap<ActionType>& map) {
                return do_choose<NextActionFinder>(current_vertex, map);
            }
        };

        // TODO(Matthew): alternative strategies like distance reduction preference (powers of).
    }
}
namespace halgo = hemlock::algorithm;

#endif // __hemlock_algorithm_acs_action_choice_strategy_action_choice_strategy_hpp

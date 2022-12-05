#ifndef __hemlock_algorithm_acs_action_choice_strategy_default_choice_strategy_hpp
#define __hemlock_algorithm_acs_action_choice_strategy_default_choice_strategy_hpp

#include "algorithm/acs/action_choice_strategy/action_choice_strategy.hpp"

namespace hemlock {
    namespace algorithm {
        template <typename ActionType>
        class DefaultChoiceStrategy :
            public ActionChoiceStrategyBase<
                ActionType,
                DefaultChoiceStrategy<ActionType>> {
        public:
            template <typename NextActionFinder>
            std::pair<bool, VertexDescriptor<ActionType>> choose(
                Ant<VertexDescriptor<ActionType>>& ant,
                f32                                exploitation_factor,
                VertexDescriptor<ActionType>       current_vertex,
                const GraphMap<ActionType>&        map
            ) {
                using _Vert = VertexDescriptor<ActionType>;

                NextActionFinder action_finder = NextActionFinder(current_vertex, map);

                size_t total_candidates = action_finder.end() - action_finder.begin();
                size_t num_surviving_candidates = 0;
                f32    total_score              = 0.0f;
                f32*   cumulative_scores        = new f32[total_candidates]{};
                _Vert* candidate_vertices       = new _Vert[total_candidates];

                struct {
                    _Vert vertex = 0;
                    f32   score  = std::numeric_limits<f32>::lowest();
                } best_option;

                for (auto edge : action_finder) {
                    _Vert candidate_vertex = boost::target(edge, map.graph);

                    // TODO(Matthew): simplify if by adding a "-1"th step to
                    // previous_vertices?
                    /**
                     * If ant has just visited the candidate vertex, then reject it as
                     * a candidate.
                     */
                    if (ant.steps_taken > 0
                        && ant.previous_vertices[ant.steps_taken - 1]
                               == candidate_vertex)
                        continue;

                    f32 score = map.edge_weight_map[edge];

                    /**
                     * If this vertex has the best score so far, set it as best
                     * option.
                     */
                    if (score > best_option.score) {
                        best_option.vertex = candidate_vertex;
                        best_option.score  = score;
                    }

                    /**
                     * Increment total score of all candidates and add new cumulative.
                     */
                    total_score += score;

                    cumulative_scores[num_surviving_candidates]  = total_score;
                    candidate_vertices[num_surviving_candidates] = candidate_vertex;

                    /**
                     * We have found a new candidate, increment count.
                     */
                    num_surviving_candidates += 1;
                }

                /**
                 * If no candidates are found, then just send the ant back to where it
                 * came from.
                 */
                if (num_surviving_candidates == 0) {
                    return { false, {} };
                }

                auto rand = [](f32 min, f32 max) {
                    std::random_device                  rand_dev;
                    std::mt19937                        generator(rand_dev());
                    std::uniform_real_distribution<f32> distribution(min, max);

                    return distribution(generator);
                };

                /**
                 * Decide if we should "exploit" (i.e., take best possible path
                 * according to scores of edges).
                 *
                 * Note that on first iteration we do no exploitation.
                 */
                f32 exploitation_val = rand(0.0f, 1.0f);
                if (exploitation_val < exploitation_factor) {
                    return { true, best_option.vertex };
                }

                // If we get here, then this ant is exploring.

                /**
                 * Choose which of the candidates to send ant to with probability in
                 * proportion to the score of each of the candidates. Bug if we
                 * somehow can't make a choice.
                 */
                f32 choice_val = rand(0.0f, total_score);
                for (size_t choice_idx = 0; choice_idx < num_surviving_candidates;
                     ++choice_idx)
                {
                    if (choice_val <= cumulative_scores[choice_idx])
                        return { true, candidate_vertices[choice_idx] };
                }

                debug_printf(
                    "Error: could not decide where to send ant, check maths of "
                    "DefaultChoiceStrategy!"
                );

                return { false, {} };
            }
        };
    }  // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_action_choice_strategy_default_choice_strategy_hpp

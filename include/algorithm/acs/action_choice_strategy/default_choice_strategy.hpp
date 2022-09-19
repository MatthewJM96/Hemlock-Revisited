#ifndef __hemlock_algorithm_acs_action_choice_strategy_default_choice_strategy_hpp
#define __hemlock_algorithm_acs_action_choice_strategy_default_choice_strategy_hpp

#include "algorithm/acs/action_choice_strategy/action_choice_strategy.hpp"

namespace hemlock {
    namespace algorithm {
        template <typename ActionType>
        class DefaultChoiceStrategy : public ActionChoiceStrategyBase<ActionType> {
        protected:
            template <typename NextActionFinder>
            std::pair<bool, VertexDescriptor<ActionType>> do_choose(
                Ant<VertexDescriptor<ActionType>>& ant,
                                               f32 exploitation_factor,
                      VertexDescriptor<ActionType> current_vertex,
                       const GraphMap<ActionType>& map
            ) {
                NextActionFinder action_finder = NextActionFinder(current_vertex, map);

                size_t total_candidates  = action_finder.end() - action_finder.begin();
                size_t num_candidates    = 0;
                   f32 total_score       = 0.0f;
                  f32* cumulative_scores = new f32[total_candidates]{};
                struct {
                    VertexDescriptor<ActionType>    vertex = 0;
                    f32                             score  = std::numeric_limits<f32>::lowest();
                } best_option;

                for (auto edge : action_finder) {
                    VertexDescriptor<ActionType> candidate_vertex = boost::target(edge, map.graph);

                    // TODO(Matthew): do we want to reject all cases of a vertex revisited?
                    /**
                     * If ant has just visited the candidate vertex, then reject it as a candidate.
                     */
                    if (ant.steps_taken > 0 && ant.previous_vertices[ant.steps_taken - 1] == candidate_vertex) continue;

                    // TODO(Matthew): 1000.0f arbitrary, just want more possible values to work with.
                    //                Is there a better solution?
                    f32 score = 1000.0f / map.edge_weight_map[edge];

                    /**
                     * If this vertex has the best score so far, set it as best option.
                     */
                    if (score > best_option.score) {
                        best_option.vertex = candidate_vertex;
                        best_option.score  = score;
                    }

                    /**
                     * Increment total score of all candidates and add new cumulative.
                     */
                    total_score += score;
                    cumulative_scores[num_candidates] = total_score;

                    /**
                     * We have found a new candidate, increment count.
                     */
                    num_candidates += 1;
                }

                /**
                 * If no candidates are found, then just send the ant back to where it came from.
                 */
                if (num_candidates == 0) {
                    return {false, 0};
                }

                auto rand = [](f32 min, f32 max) {
                    std::random_device rand_dev;
                    std::mt19937 generator(rand_dev());
                    std::uniform_real_distribution<f32> distribution(min, max);

                    return distribution(generator);
                };

                /**
                 * Decide if we should "exploit" (i.e., take best possible path according to scores of edges).
                 *
                 * Note that on first iteration we do no exploitation.
                 */
                f32 exploitation_val = rand(0.0f, 1.0f);
                if (exploitation_val < exploitation_factor) {
                    return {true, best_option.vertex};
                }

                // If we get here, then this ant is exploring.

                /**
                 * Choose which of the candidates to send ant to with probability in
                 * proportion to the score of each of the candidates. Bug if we somehow
                 * can't make a choice.
                 */
                f32 choice_val = rand(0.0f, total_score);
                for (size_t choice_idx = 0; choice_idx < num_candidates; ++choice_idx) {
                    if (choice_val <= cumulative_scores[choice_idx]) return {true, boost::target(action_finder.begin() + choice_idx, map.graph)};
                }

                debug_printf("Error: could not decide where to send ant, check maths of DefaultChoiceStrategy!");
            }
        };
    }
}
namespace halgo = hemlock::algorithm;

#endif // __hemlock_algorithm_acs_action_choice_strategy_default_choice_strategy_hpp

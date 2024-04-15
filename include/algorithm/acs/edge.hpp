#ifndef __hemlock_algorithm_acs_edge_hpp
#define __hemlock_algorithm_acs_edge_hpp

namespace hemlock {
    namespace algorithm {
        namespace acs {
            template <f32 LengthExponent, typename EdgeType>
            f32 calculate_edge_score(const EdgeType& edge) {
                // TODO(Matthew): Implement. In reference to the ACS impl I have, we
                //                should apply the local updating rule only as ants
                //                visit an edge so the iteration number is not needed.
                f32 pheromone = get_pheromone_at_edge(edge);

                // TODO(Matthew): Implement.
                f32 length = get_edge_length(edge);

                return pheromone * glm::pow(length, LengthExponent);
            }

            template <typename EdgeType>
            f32 calculate_edge_score(const EdgeType& edge, f32 length_exponent) {
                // TODO(Matthew): Implement. In reference to the ACS impl I have, we
                //                should apply the local updating rule only as ants
                //                visit an edge so the iteration number is not needed.
                f32 pheromone = get_pheromone_at_edge(edge);

                // TODO(Matthew): Implement.
                f32 length = get_edge_length(edge);

                return pheromone * glm::pow(length, length_exponent);
            }

            // TODO(Matthew): need to figure out if we want to ever support saying how
            //                many out edges can occur at runtime. Alternatively if the
            //                extra compute to do this calculation (at worst) twice over
            //                is worth avoiding an allocation and this geometric
            //                constraint.
            template <f32 LengthExponent, size_t MaxOutEdges, typename EdgeIerator>
            void calculate_edge_scores(
                EdgeIerator                   edges,
                std::array<f32, MaxOutEdges>& scores,
                f32&                          total_score,
                size_t&                       edge_count
            ) {
                // TODO(Matthew): can we rephrase to use std::foreach parallel execution
                edge_count  = 0;
                total_score = 0.0f;
                for (const auto& edge : edges) {
                    scores[edge_count]  = calculate_edge_score<LengthExponent>(edge);
                    total_score        += scores[edge_count];
                    edge_count         += 1;
                }
            }

            template <size_t MaxOutEdges, typename EdgeIerator>
            void calculate_edge_scores(
                EdgeIerator                   edges,
                std::array<f32, MaxOutEdges>& scores,
                f32&                          total_score,
                size_t&                       edge_count,
                f32                           length_exponent
            ) {
                // TODO(Matthew): can we rephrase to use std::foreach parallel execution
                edge_count  = 0;
                total_score = 0.0f;
                for (const auto& edge : edges) {
                    scores[edge_count]  = calculate_edge_score(edge, length_exponent);
                    total_score        += scores[edge_count];
                    edge_count         += 1;
                }
            }

            template <f32 LengthExponent, typename EdgeIerator>
            void calculate_edge_scores(
                EdgeIerator edges,
                f32&        total_score,
                size_t&     best_index,
                size_t&     edge_count
            ) {
                // TODO(Matthew): can we rephrase to use std::foreach parallel execution
                edge_count     = 0;
                f32 best_score = std::numeric_limits<f32>::min();
                best_index     = 0;
                total_score    = 0.0f;
                for (const auto& edge : edges) {
                    f32 score = calculate_edge_score<LengthExponent>(edge);

                    if (score > best_score) {
                        best_index = edge_count;
                        best_score = score;
                    }

                    total_score += score;
                    edge_count  += 1;
                }
            }

            template <typename EdgeIerator>
            void calculate_edge_scores(
                EdgeIerator edges,
                f32&        total_score,
                size_t&     best_index,
                size_t&     edge_count,
                f32         length_exponent
            ) {
                // TODO(Matthew): can we rephrase to use std::foreach parallel execution
                edge_count     = 0;
                f32 best_score = std::numeric_limits<f32>::min();
                best_index     = 0;
                total_score    = 0.0f;
                for (const auto& edge : edges) {
                    f32 score = calculate_edge_score(edge, length_exponent);

                    if (score > best_score) {
                        best_index = edge_count;
                        best_score = score;
                    }

                    total_score += score;
                    edge_count  += 1;
                }
            }
        }  // namespace acs
    }      // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_edge_hpp

#ifndef __hemlock_algorithm_acs_state_pheromone_hpp
#define __hemlock_algorithm_acs_state_pheromone_hpp

namespace hemlock {
    namespace algorithm {
        namespace acs {
            /**
             * @brief State required for any ACS algorithm to apply pheromone updates.
             */
            struct PhermoneStatePart {
                /**
                 * @brief Parameterises local and global pheromone update rules.
                 */
                struct {
                    /** @brief The increment applied in pheromone update. */
                    f32 increment;
                    /** @brief The evaporation applied in pheromone update. */
                    f32 evaporation;
                } local{ 0.01f, 0.1f }, global{ 10.0f, 0.1f };
            };

            /**
             * @brief Stores the pheromones on edges.
             *
             * TODO(Matthew): do we want to use this anywhere? In the case of using a
             *                graph structure, we can annotate edges using the graph
             *                API. What other kinds of data structure might we use in
             *                place of a graph?
             *                  in general, we want to solve the issue of allowing for
             *                  multiple pheromone maps that are held for separate
             *                  colonies, that may have no, some, or total overlap with
             *                  one another. That is to say, we want to only pay, at
             *                  least at some resolution (chunk, e.g.), for where each
             *                  colony operates.
             */
            template <typename VertexType>
            using PheromoneMap
                = std::unordered_map<VertexType, std::unordered_map<VertexType, f32>>;
        }  // namespace acs
    }      // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_state_pheromone_hpp

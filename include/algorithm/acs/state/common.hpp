#ifndef __hemlock_algorithm_acs_state_common_hpp
#define __hemlock_algorithm_acs_state_common_hpp

namespace hemlock {
    namespace algorithm {
        namespace acs {
            /**
             * @brief Common state required for any ACS algorithm that likely all wants
             * to be either available for runtime modification or compile-time
             * specified.
             */
            struct CommonStatePart {
                /** @brief The maximum number of iterations to perform. */
                size_t max_iterations = 10;

                /** @brief The number of ants in a colony. */
                size_t ant_count = 10;

                /**
                 * @brief Probability that an ant will exploit the current best path
                 * available to it, otherwise choosing a path according to a probability
                 * distribution built out of the same measure of path fitness over all
                 * available paths.
                 */
                f32 exploitation_factor = 0.4f;

                /**
                 * @brief The exponent a path's length is raised to in the path fitness
                 * function.
                 */
                f32 length_exponent = 1.0f;

                /**
                 * @brief Whether to do backstepping. This is done when an ant reaches
                 * a dead end in the graph, in which case it backsteps until a new
                 * option is available to it to explore.
                 *
                 * TODO(Matthew): can we do backstepping with teleportation instead, by
                 *                tracking at what step the ant last had more than one
                 *                novel option for direction? It would be simply better
                 *                than what we have right now...
                 */
                bool do_backstepping = true;

                /** @brief The degree of path change that is satisfactory. */
                f32 break_on_path_change = 0.0f;
                /**
                 * @brief The number of iterations in which satisfactory path change
                 * occurs on which to early break the path search.
                 */
                size_t break_on_iterations = std::numeric_limits<size_t>::max();

                /**
                 * @brief Common debug state required for any convergent ACS algorithm.
                 */
                struct {
                    /** @brief Whether debugging should occur. */
                    bool on = false;
                    /** @brief Produce debug information every n iterations. */
                    size_t n_iterations = 1;
                    /** @brief Produce debug information every n steps. */
                    size_t n_steps = 1;
                } debug;
            };
        }  // namespace acs
    }      // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_state_common_hpp

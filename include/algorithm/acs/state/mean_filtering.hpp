#ifndef __hemlock_algorithm_acs_state_mean_filtering_hpp
#define __hemlock_algorithm_acs_state_mean_filtering_hpp

namespace hemlock {
    namespace algorithm {
        namespace acs {
            /**
             * @brief State required for any ACS algorithm to apply the mean filtering
             * rule.
             */
            struct MeanFilteringStatePart {
                /**
                 * @brief The "neighbour of" order of the mean filtering rule. For an
                 * order K, mean filtering is applied out to the Kth nearest neighbours
                 * of a node.
                 */
                size_t order = 0;
                /**
                 * @brief The entropy value at or below which the mean filtering rule is
                 * triggered. A negative value guarantees mean filtering never occurs.
                 */
                f32 trigger = -1.0f;
            };
        }  // namespace acs
    }      // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_state_mean_filtering_hpp

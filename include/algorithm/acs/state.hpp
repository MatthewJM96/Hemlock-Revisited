#ifndef __hemlock_algorithm_acs_state_hpp
#define __hemlock_algorithm_acs_state_hpp

#include "graph/state.hpp"

namespace hemlock {
    namespace algorithm {
        struct ACSConfig {
            size_t max_iterations
                = 10;  //< The maximum number of iterations to perform.
            f32 break_on_path_change
                = 0.0f;  //< The degree of path change that is  satisfactory.
            size_t break_on_iterations
                = 3;  //< The number of iterations in which satisfactory path change
                      // occurs on which to break the path search.
            size_t max_steps
                = 100;  //< The maximum number of steps to allow any one ant to take.
            size_t ant_count = 10;  //< The number of ants to use in path finding.

            struct {
                f32 base  = 0.4f;  //< The base exploitation factor.
                f32 coeff = 0.4f;  //< The coefficient used in applying entropy-based
                                   // dynamic exploitation factor. Setting this to zero
                                   // turns off dynamic exploitation factor.
                f32 exp = 2.0f;  //< The exponent used in applying entropy-based dynamic
                                 // exploitation factor.
            } exploitation = {};

            struct {
                size_t order
                    = 0;  //< The "neighbour of" order to go out to to calculate mean.
                          // Setting this to zero turns off mean filtering.
                f32 trigger
                    = 0.0f;  //< The entropy value for which to trigger mean filtering.
            } mean_filtering = {};

            struct {
                f32 increment
                    = 0.01f;  //< The increment applied in local pheromone update.
                f32 evaporation
                    = 0.1f;  //< The evaporation applied in local pheromone update.
            } local = {};

            struct {
                f32 increment
                    = 10.0f;  //< The increment applied in global pheromone update.
                f32 evaporation
                    = 0.1f;  //< The evaporation applied in global pheromone update.
            } global = {};

            struct {
                bool   on           = false;  //< Whether to debug the ACS.
                size_t n_iterations = 1;  //< Create heatframes in every n iterations.
                size_t n_steps
                    = 1;  //< Create a heatframe every n steps in an iteration.
            } debug = {};
        };

        template <typename Candidate, typename Node, bool IsWeighted>
        concept ACSDistanceCalculator
            = requires (Candidate s, GraphMap<Node, IsWeighted> g, Node n) {
                  {
                      s.operator()(g, n, n, n)
                      } -> std::same_as<f32>;
              };

        template <typename Node, bool IsWeighted>
        struct NullACSDistanceCalculator {
            f32
            operator()(const GraphMap<Node, IsWeighted>&, const Node&, const Node&, const Node&)
                const {
                return 0.0f;
            }
        };

        template <typename VertexType>
        struct Ant {
            bool   found_food   = false;
            bool   did_backstep = false;
            size_t steps_taken  = 0;
            size_t group        = 0;

            VertexType* previous_vertices = nullptr;
            VertexType  current_vertex    = {};
        };

        template <typename VertexType, size_t AntCount>
        struct AntGroup {
            Ant<VertexType>* ants[AntCount] = {};
            size_t           size           = 0;
        };
    }  // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_state_hpp

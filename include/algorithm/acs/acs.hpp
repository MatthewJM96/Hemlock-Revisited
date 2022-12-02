#ifndef __hemlock_algorithm_acs_acs_hpp
#define __hemlock_algorithm_acs_acs_hpp

#include "algorithm/graph.hpp"
#include "algorithm/next_action_finder.hpp"
#include "algorithm/acs/action_choice_strategy/default_choice_strategy.hpp"

namespace hemlock {
    namespace algorithm {
        template <typename VertexDescriptor>
        struct Ant {
            bool   alive        = true;
            bool   found_food   = false;
            size_t steps_taken  = 0;
            size_t group        = 0;

            VertexDescriptor* previous_vertices;
            VertexDescriptor  current_vertex;
        };

        template < typename VertexDescriptor, size_t AntCount>
        struct AntGroup {
            Ant<VertexDescriptor>* ants[AntCount] = {};
            size_t  size = 0;
        };

        // TODO(Matthew): Differentiate edge weight and edge pheromone...
        // TODO(Matthew): Don't necessarily want to provide a graph map - scenarios exist where
        //                no sharing can or will occur and thus it may be faster to deal simply
        //                with VertexData.
        // TODO(Matthew): Can we drop pheromone proportionate to some metric (naively degree of
        //                distance reduction on path) so long as no ant has yet actually reached
        //                the target?
        // TODO(Matthew): Allow a variation where the destination is a comparator. Note that here
        //                we will need to allow providing an optional metric for determining
        //                distance, and where not given the algorithm will have to disable some
        //                components.
        // TODO(Matthew): Implement backtracking, it's the only sane option! Track groups by path
        //                length & current vertex.
        template <typename VertexData, typename NextActionFinder = NextActionFromGraphFinder<VertexData>, typename VertexChoiceStrategy = DefaultChoiceStrategy<VertexData>>
        class BasicACS {
        public:
            BasicACS();
            ~BasicACS() { /* Empty. */ }

            BasicACS& set_max_iterations(size_t max_iterations);
            BasicACS& set_break_on_path_change(f32 break_on_path_change);
            BasicACS& set_break_on_iterations(size_t break_on_iterations);
            // BasicACS& set_max_steps(size_t max_steps);
            // BasicACS& set_ant_count(size_t ant_count);
            BasicACS& set_exploitation_base(f32 exploitation_base);
            BasicACS& set_exploitation_coeff(f32 exploitation_coeff);
            BasicACS& set_exploitation_exponent(f32 exploitation_exponent);
            BasicACS& set_do_mean_filtering(bool do_mean_filtering);
            BasicACS& set_mean_filtering_order(size_t mean_filtering_order);
            BasicACS& set_mean_filtering_trigger(f32 mean_filtering_trigger);
            BasicACS& set_local_increment(f32 local_increment);
            BasicACS& set_local_evaporation(f32 local_evaporation);
            BasicACS& set_global_increment(f32 global_increment);
            BasicACS& set_global_evaporation(f32 global_evaporation);

            template <size_t AntCount, size_t MaxSteps>
            void find_path(GraphMap<VertexData>& map, VertexData source, VertexData destination, VertexData*& path, size_t& path_length);
        protected:
            size_t  m_max_iterations;               //< The maximum number of iterations to perform.
            f32     m_break_on_path_change;         //< The degree of path change that is satisfactory.
            size_t  m_break_on_iterations;          //< The number of iterations in which satisfactory path
                                                    //  change occurs on which to break the path search.
            // size_t  m_max_steps;                    //< The maximum number of steps to allow any one ant to take.
            // size_t  m_ant_count;                    //< The number of ants to use in path finding.
            f32     m_exploitation_base;            //< The base exploitation factor.
            f32     m_exploitation_coeff;           //< The coefficient used in applying entropy-based dynamic exploitation factor.
                                                    //  Setting this to zero turns off dynamic exploitation factor.
            f32     m_exploitation_exponent;        //< The exponent used in applying entropy-based dynamic exploitation factor.
            bool    m_do_mean_filtering;            //< Whether to do mean filtering at all.
            size_t  m_mean_filtering_order;         //< The "neighbour of" order to go out to to calculate mean.
            // TODO(Matthew): More intelligent use of mean filtering versus dynamic exploitation (first is a very
            //                local increase of exploration where second is global).
            f32     m_mean_filtering_trigger;       //< The entropy value for which to trigger mean filtering.
            f32     m_local_increment;              //< The increment applied in local pheromone update.
            f32     m_local_evaporation;            //< The evaporation applied in local pheromone update.
            f32     m_global_increment;             //< The increment applied in global pheromone update.
            f32     m_global_evaporation;           //< The evaporation applied in global pheromone update.
        };
    }
}
namespace halgo = hemlock::algorithm;

#include "algorithm/acs/acs.inl"

#endif // __hemlock_algorithm_acs_acs_hpp

#ifndef __hemlock_algorithm_debug_acs_acs_hpp
#define __hemlock_algorithm_debug_acs_acs_hpp

#include "debug/heatmap.h"

namespace hemlock {
    namespace algorithm {
        namespace debug {
            template <typename Node>
            using NodeTo2DCoord = std::function<std::tuple<size_t, size_t>(Node)>;

            template <typename Node, bool IsWeighted>
            class ACSHeatmap2D {
                using _Ant = Ant<VertexDescriptor<Node, IsWeighted>, Node, IsWeighted>;
            public:
                ACSHeatmap2D();

                ~ACSHeatmap2D() { /* Empty. */
                }

                ACSHeatmap2D& set_vertex_to_2d_coord(NodeTo2DCoord<Node> get_2d_coord);
                ACSHeatmap2D& set_protoheatmap(heatmap_t* protoheatmap);

                void initialise_heatmaps(size_t max_steps, size_t max_iterations);

                void create_heatmaps(
                    _Ant* ants, size_t ant_count, GraphMap<Node, IsWeighted>& map
                );

                void dispose_heatmaps();

                void print_heatmap_frames(const std::string& tag);
            protected:
                void create_pheromone_heatmap_frame(GraphMap<Node, IsWeighted>& map);

                void create_ant_count_heatmap_frame(
                    _Ant* ants, size_t ant_count, GraphMap<Node, IsWeighted>& map
                );

                NodeTo2DCoord<Node> m_get_2d_coord;

                size_t m_heatmap_frame_count;

                heatmap_t* m_protoheatmap;
                heatmap_t* m_pheromone_heatmaps;
                heatmap_t* m_ant_count_heatmaps;
            };
        }  // namespace debug
        namespace deb = debug;
    }  // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#include "acs.inl"

#endif  // __hemlock_algorithm_debug_acs_acs_hpp

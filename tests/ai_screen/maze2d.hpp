#ifndef __hemlock_tests_ai_screen_maze2d_hpp
#define __hemlock_tests_ai_screen_maze2d_hpp

#include "dimension.hpp"

namespace map {
    namespace maze2d {
        using namespace dimension;

        using IndexCoordMap = std::unordered_map<size_t, std::tuple<size_t, size_t>>;

        struct Map {
            char*           map;
            Map2DDimensions dims;
            size_t          start_idx;
            size_t          finish_idx;
            size_t          solution_length;
            IndexCoordMap   index_coord_map;
            heatmap_t*      protoheatmap;
        };

        Map load_map(std::string map_filepath, Map2DDimensions dimensions);

        void print_map(const Map& map);

        halgo::GraphMap<size_t> map_to_graph(Map map, float initial_weight);
    };  // namespace maze2d
};      // namespace map

#include "maze2d.inl"

#endif  // __hemlock_tests_ai_screen_maze2d_hpp

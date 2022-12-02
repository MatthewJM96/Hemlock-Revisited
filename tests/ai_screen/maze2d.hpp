#ifndef __hemlock_tests_ai_screen_maze2d_hpp
#define __hemlock_tests_ai_screen_maze2d_hpp

#include "dimension.hpp"

namespace map {
    namespace maze2d {
        using namespace dimension;

        struct Map {
            char*           map;
            Map2DDimensions dims;
            size_t          start_idx;
            size_t          finish_idx;
            size_t          solution_length;
        };

        Map load_map(std::string map_filepath, Map2DDimensions dimensions);

        void print_map(const Map& map);

        halgo::GraphMap<size_t> map_to_graph(Map map, float initial_weight);
    };
};

#include "maze2d.inl"

#endif // __hemlock_tests_ai_screen_maze2d_hpp

#include "algorithm/debug/debug.hpp"
#include "maze2d.hpp"

void __do_maze_graph_test(size_t map_dim, size_t count) {
    size_t half_dim = ((map_dim - 1) / 2);

    for (size_t map_idx = 0; map_idx < count; ++map_idx) {
        map::maze2d::Map map;

        std::string tag = std::to_string(half_dim) + "." + std::to_string(map_idx);

        map = map::maze2d::load_map(
            "maze/" + tag + ".solved.map", { map_dim, map_dim }
        );

        std::cout << "Map " << map_idx + 1 << " of dim " << half_dim
                  << " maps, with ideal solution length " << map.solution_length << ":"
                  << std::endl;

        halgo::GraphMap<size_t, false> graph_map
            = map::maze2d::map_to_graph(map, 0.01f);

        // size_t num_vertices = boost::num_vertices(graph_map.graph);
        // std::cout << "Num Vertices: " << num_vertices << std::endl;

        constexpr halgo::ACSConfig Config = { .max_iterations = 100,
                                              .max_steps      = 600,
                                              .ant_count      = 500,
                                              .debug          = { .on = false } };

        halgo::deb::ACSHeatmap2D<size_t, false> debugger;

        debugger.set_protoheatmap(map.protoheatmap);

        debugger.set_vertex_to_2d_coord(
            [&](size_t coord) -> std::tuple<size_t, size_t> {
                return map.index_coord_map[coord];
            }
        );

        size_t* path        = nullptr;
        size_t  path_length = 0;
        halgo::GraphACS::find_path<size_t, false, Config>(
            graph_map, map.start_idx, map.finish_idx, path, path_length, &debugger
        );
        delete[] path;

        debugger.print_heatmap_frames(tag);

        std::cout << "  solution found with length: " << path_length << std::endl
                  << std::endl;
    }
}

void do_maze_graph_test() {
    std::cout << "\n****************\n* Maze Size 15 *\n****************\n\n";

    __do_maze_graph_test(15, 10);

    std::cout << "\n****************\n* Maze Size 30 *\n****************\n\n";

    __do_maze_graph_test(31, 10);

    std::cout << "\n****************\n* Maze Size 60 *\n****************\n\n";

    __do_maze_graph_test(61, 10);
}

void __do_maze_graph_timing_test(size_t iterations, size_t map_dim, size_t map_idx) {
    size_t half_dim = ((map_dim - 1) / 2);

    map::maze2d::Map map;

    std::string tag = std::to_string(half_dim) + "." + std::to_string(map_idx);

    map = map::maze2d::load_map("maze/" + tag + ".solved.map", { map_dim, map_dim });

    std::cout << "Map " << map_idx + 1 << " of dim " << half_dim
              << " maps, with ideal solution length " << map.solution_length << ":"
              << std::endl;

    halgo::GraphMap<size_t, false> graph_map = map::maze2d::map_to_graph(map, 0.01f);

    constexpr halgo::ACSConfig Config = { .max_steps = 300, .debug = { .on = false } };

    size_t** paths = new size_t*[iterations];

    std::chrono::nanoseconds duration = {};
    for (size_t iteration = 0; iteration < iterations; ++iteration) {
        auto   start       = std::chrono::steady_clock::now();
        size_t path_length = 0;
        halgo::GraphACS::find_path<size_t, false, Config>(
            graph_map, map.start_idx, map.finish_idx, paths[iteration], path_length
        );
        duration += std::chrono::steady_clock::now() - start;
    }

    std::cout << "  solution found with average time: "
              << static_cast<f32>(duration.count()) / static_cast<f32>(iterations)
              << "ns" << std::endl
              << std::endl;

    for (size_t iteration = 0; iteration < iterations; ++iteration) {
        delete[] paths[iteration];
    }
}

void do_maze_graph_timing_test(size_t iterations, size_t map_idx = 0) {
    std::cout << "\n****************\n* Maze Size 15 *\n****************\n\n";

    __do_maze_graph_timing_test(iterations, 15, map_idx);

    // std::cout << "\n****************\n* Maze Size 30 *\n****************\n\n";

    // __do_maze_graph_timing_test(iterations, 31, map_idx);

    // std::cout << "\n****************\n* Maze Size 60 *\n****************\n\n";

    // __do_maze_graph_timing_test(iterations, 61, map_idx);
}

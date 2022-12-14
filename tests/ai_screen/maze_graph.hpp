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

        constexpr halgo::ACSConfig Config = { .debug = { .on = true } };

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
    __do_maze_graph_test(15, 10);

    __do_maze_graph_test(31, 10);

    __do_maze_graph_test(61, 10);
}

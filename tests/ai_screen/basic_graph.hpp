#ifndef __hemlock_tests_ai_basic_graph_hpp
#define __hemlock_tests_ai_basic_graph_hpp

void do_basic_graph_test() {
    halgo::GraphMap<int, false> graph_map;

    auto one   = boost::add_vertex(graph_map.graph);
    auto two   = boost::add_vertex(graph_map.graph);
    auto three = boost::add_vertex(graph_map.graph);
    auto four  = boost::add_vertex(graph_map.graph);

    graph_map.vertex_coord_map[one]   = 1;
    graph_map.vertex_coord_map[two]   = 2;
    graph_map.vertex_coord_map[three] = 3;
    graph_map.vertex_coord_map[four]  = 4;

    graph_map.coord_vertex_map[1] = one;
    graph_map.coord_vertex_map[2] = two;
    graph_map.coord_vertex_map[3] = three;
    graph_map.coord_vertex_map[4] = four;

    auto [edge_one_to_two, _1]   = boost::add_edge(one, two, graph_map.graph);
    auto [edge_two_to_three, _2] = boost::add_edge(two, three, graph_map.graph);
    auto [edge_one_to_four, _3]  = boost::add_edge(one, four, graph_map.graph);

    graph_map.pheromone_map[edge_one_to_two]   = 1.0f;
    graph_map.pheromone_map[edge_two_to_three] = 1.0f;
    graph_map.pheromone_map[edge_one_to_four]  = 1.0f;

    auto [edge_two_to_one, _a]   = boost::add_edge(two, one, graph_map.graph);
    auto [edge_three_to_two, _b] = boost::add_edge(three, two, graph_map.graph);
    auto [edge_four_to_one, _c]  = boost::add_edge(four, one, graph_map.graph);

    graph_map.pheromone_map[edge_two_to_one]   = 1.0f;
    graph_map.pheromone_map[edge_three_to_two] = 1.0f;
    graph_map.pheromone_map[edge_four_to_one]  = 1.0f;

    constexpr halgo::ACSConfig Config = { .debug = { .on = true } };

    int*   path        = nullptr;
    size_t path_length = 0;
    halgo::GraphACS::find_path<int, false, Config>(graph_map, 1, 3, path, path_length);

    std::cout << path_length << std::endl;
    for (size_t i = 0; i < path_length; ++i) {
        std::cout << path[i] << " - ";
    }
    std::cout << std::endl;
}

#endif  // __hemlock_tests_ai_basic_graph_hpp

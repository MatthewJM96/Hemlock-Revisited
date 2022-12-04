#define WALL_TILE     '#'
#define SOLUTION_TILE '+'
#define START_TILE    'S'
#define END_TILE      'E'

map::maze2d::Map
map::maze2d::load_map(std::string map_filepath, Map2DDimensions dimensions) {
    const size_t padded_dim_x = dimension::dim_to_padded_dim(dimensions.x);
    const size_t padded_dim_y = dimension::dim_to_padded_dim(dimensions.y);

    Map map;

    map.dims            = { padded_dim_x, padded_dim_y };
    map.solution_length = 0;
    map.map             = new char[map.dims.x * map.dims.y];

    // Initialise first and last row of halo map.
    for (size_t i = 0; i < padded_dim_x; ++i) {
        map.map[i]                                     = '#';
        map.map[padded_dim_x * (padded_dim_y - 1) + i] = '#';
    }

    std::ifstream map_file(map_filepath);

    // For each line in the map file, copy in its contents,
    // padding with a wall tile on each side.
    size_t      row = 0;
    std::string line;
    while (std::getline(map_file, line)) {
        map.map[(row + 1) * padded_dim_x]     = WALL_TILE;
        map.map[(row + 2) * padded_dim_x - 1] = WALL_TILE;

        // Iterate over the "inner" (non-halo) width - that is,
        // the part we actually load from the map file.
        for (size_t col = 0; col < dimensions.x; ++col) {
            size_t halo_idx = (row + 1) * padded_dim_x + col + 1;

            map.map[halo_idx] = line[col];

            if (line[col] == START_TILE) {
                map.start_idx = halo_idx;
            } else if (line[col] == END_TILE) {
                map.finish_idx = halo_idx;
            } else if (line[col] == SOLUTION_TILE) {
                map.solution_length += 1;
            }
        }

        ++row;
    }

    // Increment solution length one more time to get to end tile.
    map.solution_length += 1;

    return map;
}

void map::maze2d::print_map(const Map& map) {
    for (size_t i = 0; i < map.dims.y; ++i) {
        for (size_t j = 0; j < map.dims.x; ++j) {
            size_t idx = i * map.dims.x + j;

            std::cout << map.map[idx] << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\n";
}

halgo::GraphMap<size_t> map::maze2d::map_to_graph(Map map, float initial_weight) {
    using _VertexDescriptor = halgo::VertexDescriptor<size_t>;

    halgo::GraphMap<size_t> graph_map;

    std::unordered_map<size_t, bool> nodes_visited;

    std::list<size_t> nodes_to_visit;
    nodes_to_visit.push_back(map.start_idx);

    auto do_adjacent_node_visit = [&](size_t target_node, size_t origin_node) {
        if (map.map[target_node] == WALL_TILE) return;

        // Ugly!
        try {
            if (nodes_visited.at(target_node)) return;
        } catch (std::exception&) { /* Empty */
        }

        _VertexDescriptor v_target;
        try {
            v_target = graph_map.coord_vertex_map.at(target_node);
        } catch (std::exception&) {
            v_target = boost::add_vertex(graph_map.graph);
        }
        _VertexDescriptor v_origin = graph_map.coord_vertex_map[origin_node];

        auto [e_o_to_t, e_o_to_t_made]
            = boost::add_edge(v_origin, v_target, graph_map.graph);
        auto [e_t_to_o, e_t_to_o_made]
            = boost::add_edge(v_target, v_origin, graph_map.graph);

        // Bool flags should always be true, better check would fail
        // graph building on a false.
        if (e_o_to_t_made) graph_map.edge_weight_map[e_o_to_t] = initial_weight;
        if (e_t_to_o_made) graph_map.edge_weight_map[e_t_to_o] = initial_weight;

        graph_map.vertex_coord_map[v_target]    = target_node;
        graph_map.coord_vertex_map[target_node] = v_target;

        if (std::find(nodes_to_visit.begin(), nodes_to_visit.end(), target_node)
            == nodes_to_visit.end())
            nodes_to_visit.push_back(target_node);
    };

    _VertexDescriptor origin                  = boost::add_vertex(graph_map.graph);
    graph_map.vertex_coord_map[origin]        = map.start_idx;
    graph_map.coord_vertex_map[map.start_idx] = origin;

    while (nodes_to_visit.size() > 0) {
        size_t number_nodes_to_visit_in_round = nodes_to_visit.size();

        auto   it                           = nodes_to_visit.begin();
        size_t nodes_left_to_visit_in_round = number_nodes_to_visit_in_round;
        while (nodes_left_to_visit_in_round > 0) {
            size_t current_node_idx = *it;

            size_t row_idx = std::floor(
                static_cast<float>(current_node_idx) / static_cast<float>(map.dims.x)
            );
            size_t col_idx = current_node_idx % map.dims.x;

            // do_adjacent_node_visit((row_idx - 1) * map.dims.x + col_idx - 1,
            // current_node_idx);
            do_adjacent_node_visit(
                (row_idx - 1) * map.dims.x + col_idx, current_node_idx
            );
            // do_adjacent_node_visit((row_idx - 1) * map.dims.x + col_idx + 1,
            // current_node_idx);
            do_adjacent_node_visit(
                row_idx * map.dims.x + col_idx + 1, current_node_idx
            );
            // do_adjacent_node_visit((row_idx + 1) * map.dims.x + col_idx + 1,
            // current_node_idx);
            do_adjacent_node_visit(
                (row_idx + 1) * map.dims.x + col_idx, current_node_idx
            );
            // do_adjacent_node_visit((row_idx + 1) * map.dims.x + col_idx - 1,
            // current_node_idx);
            do_adjacent_node_visit(
                row_idx * map.dims.x + col_idx - 1, current_node_idx
            );

            nodes_visited[current_node_idx] = true;

            --nodes_left_to_visit_in_round;
            ++it;
        }

        nodes_to_visit.erase(
            nodes_to_visit.begin(),
            std::next(nodes_to_visit.begin(), number_nodes_to_visit_in_round)
        );
    }

    return graph_map;
}

#undef WALL_TILE
#undef SOLUTION_TILE
#undef START_TILE
#undef END_TILE

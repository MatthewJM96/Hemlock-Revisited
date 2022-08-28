template <typename VertexCoordType>
void halgo::reset_acs_graph_map_best_path(ACSGraphMap<VertexCoordType> map) {
    for (auto edge : boost::make_iterator_range(boost::edges(map.graph))) {
        map.edge_in_path_map[edge] = false;
    }
}

template <typename VertexCoordType>
void halgo::reset_acs_graph_map(ACSGraphMap<VertexCoordType> map) {
    for (auto edge : boost::make_iterator_range(boost::edges(map.graph))) {
        map.edge_weight_map[edge]  = 0.0f;
        map.edge_in_path_map[edge] = false;
    }
}

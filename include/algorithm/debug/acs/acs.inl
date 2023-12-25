template <typename Node, bool IsWeighted>
halgo::deb::ACSHeatmap2D<Node, IsWeighted>::ACSHeatmap2D() :
    m_heatmap_frame_count(0),
    m_protoheatmap(nullptr),
    m_pheromone_heatmaps(nullptr),
    m_ant_count_heatmaps(nullptr) {
    // Empty.
}

template <typename Node, bool IsWeighted>
halgo::deb::ACSHeatmap2D<Node, IsWeighted>&
halgo::deb::ACSHeatmap2D<Node, IsWeighted>::set_vertex_to_2d_coord(
    NodeTo2DCoord<Node> get_2d_coord
) {
    m_get_2d_coord = get_2d_coord;

    return *this;
}

template <typename Node, bool IsWeighted>
halgo::deb::ACSHeatmap2D<Node, IsWeighted>&
halgo::deb::ACSHeatmap2D<Node, IsWeighted>::set_protoheatmap(heatmap_t* protoheatmap) {
    m_protoheatmap = protoheatmap;

    return *this;
}

template <typename Node, bool IsWeighted>
void halgo::deb::ACSHeatmap2D<Node, IsWeighted>::initialise_heatmaps(
    size_t max_steps, size_t max_iterations
) {
    if (m_protoheatmap == nullptr) return;

    // Allocate all heatmap buffers and the heatmaps themselves as voxels and divy out.
    f32* buffers = new f32
        [max_steps * max_iterations * 2 * m_protoheatmap->w * m_protoheatmap->h]();

    heatmap_t* heatmaps = new heatmap_t[max_steps * max_iterations * 2]();

    for (size_t heatmap_idx = 0; heatmap_idx < max_steps * max_iterations * 2;
         heatmap_idx++)
    {
        heatmaps[heatmap_idx].w = m_protoheatmap->w;
        heatmaps[heatmap_idx].h = m_protoheatmap->h;
        heatmaps[heatmap_idx].buf
            = &buffers[heatmap_idx * m_protoheatmap->w * m_protoheatmap->h];
    }

    m_pheromone_heatmaps = &heatmaps[0];
    m_ant_count_heatmaps = &heatmaps[max_steps * max_iterations];
}

template <typename Node, bool IsWeighted>
void halgo::deb::ACSHeatmap2D<Node, IsWeighted>::create_heatmaps(
    _Ant* ants, size_t ant_count, GraphMap<Node, IsWeighted>& map
) {
    create_pheromone_heatmap_frame(map);

    create_ant_count_heatmap_frame(ants, ant_count, map);

    m_heatmap_frame_count += 1;
}

template <typename Node, bool IsWeighted>
void halgo::deb::ACSHeatmap2D<Node, IsWeighted>::dispose_heatmaps() {
    if (m_pheromone_heatmaps == nullptr) return;

    delete[] m_pheromone_heatmaps[0].buf;
    delete[] m_pheromone_heatmaps;

    m_pheromone_heatmaps = nullptr;
    m_ant_count_heatmaps = nullptr;
}

template <typename Node, bool IsWeighted>
void halgo::deb::ACSHeatmap2D<Node, IsWeighted>::print_heatmap_frames(
    const std::string& tag
) {
    hdeb::print_heatmaps(
        m_pheromone_heatmaps, m_heatmap_frame_count, "maze/results", tag + ".pheromone"
    );

    hdeb::print_heatmaps(
        m_ant_count_heatmaps, m_heatmap_frame_count, "maze/results", tag + ".ant_count"
    );
}

template <typename Node, bool IsWeighted>
void halgo::deb::ACSHeatmap2D<Node, IsWeighted>::create_pheromone_heatmap_frame(
    GraphMap<Node, IsWeighted>& map
) {
    if (m_protoheatmap == nullptr) return;

    heatmap_t* heatmap = &m_pheromone_heatmaps[m_heatmap_frame_count];

    // For each vertex in graph, sum up pheromone on edges leading into it and add
    // a point weighted by this to the heatmap.
    for (auto vertex : boost::make_iterator_range(boost::vertices(map.graph))) {
        f32 net_pheromone_into_vertex = 0.0f;
        for (auto edge : boost::make_iterator_range(boost::in_edges(vertex, map.graph)))
            net_pheromone_into_vertex += map.pheromone_map[edge];

        auto [x, y] = m_get_2d_coord(map.vertex_coord_map[vertex]);

        heatmap_add_weighted_point(heatmap, x, y, net_pheromone_into_vertex);
    }

    // Apply protoheatmap to this heatmap frame factoring in strongest pheromone signal.
    size_t pixel_count = m_protoheatmap->w * m_protoheatmap->h;
    for (size_t pixel_idx = 0; pixel_idx < pixel_count; ++pixel_idx) {
        heatmap->buf[pixel_idx] += m_protoheatmap->buf[pixel_idx] * heatmap->max;
    }
}

template <typename Node, bool IsWeighted>
void halgo::deb::ACSHeatmap2D<Node, IsWeighted>::create_ant_count_heatmap_frame(
    _Ant* ants, size_t ant_count, GraphMap<Node, IsWeighted>& map
) {
    if (m_protoheatmap == nullptr) return;

    heatmap_t* heatmap = &m_ant_count_heatmaps[m_heatmap_frame_count];

    // For each ant add point to heatmap for its current location on the graph.
    for (size_t ant_idx = 0; ant_idx < ant_count; ++ant_idx) {
        auto& ant = ants[ant_idx];

        auto [x, y] = m_get_2d_coord(map.vertex_coord_map[ant.current_vertex]);

        heatmap_add_point(heatmap, x, y);
    }

    // Apply protoheatmap to this heatmap frame.
    size_t pixel_count = m_protoheatmap->w * m_protoheatmap->h;
    for (size_t pixel_idx = 0; pixel_idx < pixel_count; ++pixel_idx) {
        heatmap->buf[pixel_idx] += m_protoheatmap->buf[pixel_idx] * heatmap->max;
    }
}

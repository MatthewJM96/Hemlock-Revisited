template <typename Graph>
halgo::LoopLocator<Graph>::LoopLocator(const LoopLocator& rhs) :
    boost::default_dfs_visitor(rhs),
    m_curr_path(rhs.m_curr_path),
    m_loops(rhs.m_loops) { /* Empty. */
}

template <typename Graph>
halgo::LoopLocator<Graph>::LoopLocator(LoopLocator&& rhs) :
    boost::default_dfs_visitor(std::move(rhs)),
    m_curr_path(std::move(rhs.m_curr_path)),
    m_loops(std::move(rhs.m_loops)) { /* Empty. */
}

template <typename Graph>
void halgo::LoopLocator<Graph>::init(const Loops& loops) {
    m_loops = loops;
}

template <typename Graph>
void halgo::LoopLocator<Graph>::dispose() {
    m_root_vertex = 0;
    m_curr_path   = {};
    m_loops       = {};
}

template <typename Graph>
void halgo::LoopLocator<Graph>::tree_edge(Edge edge, const Graph&) {
    m_curr_path.push_back(edge);
}

template <typename Graph>
void halgo::LoopLocator<Graph>::back_edge(Edge edge, const Graph& graph) {
    // Store a record of the back edge as we will need to know about this to handle
    // forward/cross edges hitting a vertex that has an outbound back edge.
    // TODO(Matthew): do we need to use this here as well, actually?
    // if (m_back_edges.contains(boost::source(edge))) {
    //     m_back_edges[boost::source(edge)].emplace_back(edge);
    // } else {
    //     m_back_edges[boost::source(edge)] = Edges{ edge };
    // }

    // Find where in the current path the back edge points to, this is the beginning of
    // the loop we have just identified.
    auto it
        = std::find_if(m_curr_path.begin(), m_curr_path.end(), [&](const auto& comp) {
              return boost::target(edge, graph) == boost::source(comp, graph);
          });

    // Note that we were looking at sources of each edge in the current path traversed,
    // this doesn't capture the case that a loop is formed by virtue of a self-looping
    // vertex. We handle that here.
    if (it == m_curr_path.end()) {
        if (boost::target(edge, graph) == boost::source(edge, graph)) {
            m_loops->emplace_back(std::vector<Edge>{ edge });
        }
    } else {
        // Store the loop.
        m_loops->emplace_back(it, m_curr_path.end());
        m_loops->back().push_back(edge);
    }
}

template <typename Graph>
void halgo::LoopLocator<Graph>::start_vertex(Vertex vertex, const Graph&) {
    m_root_vertex = vertex;
}

template <typename Graph>
void halgo::LoopLocator<Graph>::finish_vertex(Vertex, const Graph&) {
    if (!m_curr_path.empty()) m_curr_path.pop_back();
}

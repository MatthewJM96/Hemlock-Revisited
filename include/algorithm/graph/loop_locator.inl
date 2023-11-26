template <typename Graph>
halgo::LoopLocator<Graph>::LoopLocator(const LoopLocator& rhs) :
    boost::default_dfs_visitor(rhs),
    m_curr_path(rhs.m_curr_path),
    m_back_edges(rhs.m_back_edges),
    m_loops(rhs.m_loops),
    m_loop_vertex_map(rhs.m_loop_vertex_map),
    m_loops_visited(rhs.m_loops_visited) { /* Empty. */
}

template <typename Graph>
halgo::LoopLocator<Graph>::LoopLocator(LoopLocator&& rhs) :
    boost::default_dfs_visitor(std::move(rhs)),
    m_curr_path(std::move(rhs.m_curr_path)),
    m_back_edges(std::move(rhs.m_back_edges)),
    m_loops(std::move(rhs.m_loops)),
    m_loop_vertex_map(std::move(rhs.m_loop_vertex_map)),
    m_loops_visited(std::move(rhs.m_loops_visited)) { /* Empty. */
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
    if (m_back_edges.contains(boost::source(edge, graph))) {
        m_back_edges[boost::source(edge, graph)].emplace_back(edge);
    } else {
        m_back_edges[boost::source(edge, graph)] = Edges{ edge };
    }

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
            m_loop_vertex_map[boost::source(edge, graph)].emplace_back(m_loops->size());
        }
    } else {
        // Store the loop.
        m_loops->emplace_back(it, m_curr_path.end());
        m_loops->back().push_back(edge);

        for (const auto& loop_edge : m_loops->back()) {
            m_loop_vertex_map[boost::source(loop_edge, graph)].emplace_back(
                m_loops->size()
            );
        }
    }
}

template <typename Graph>
void halgo::LoopLocator<Graph>::forward_or_cross_edge(Edge edge, const Graph& graph) {
    std::vector<std::vector<Edge>> new_loops;

    // If this edge links into any existing loop, we must then determine if that loop
    // somewhere else points to an ancestor in the DFS tree from the source vertex of
    // this current edge.
    for (const auto& loop : *m_loops) {
        auto loop_entry_it
            = std::find_if(loop.begin(), loop.end(), [&](const auto& comp) {
                  return boost::target(edge, graph) == boost::source(comp, graph);
              });

        // Skip to next loop if no link into this one was found.
        if (loop_entry_it == loop.end()) {
            continue;
        }

        for (; loop_entry_it != loop.end(); ++loop_entry_it) {
            if (m_curr_path.empty()) {
                if (*loop_entry_it == m_root_vertex) {
                    // We have a path that loops about root index.
                    new_loops.emplace_back(std::vector<Edge>{ edge });
                    new_loops.back().insert(
                        new_loops.back().end(), loop_entry_it, loop.end()
                    );
                }
            } else {
                auto loop_exit_it = std::find_if(
                    m_curr_path.begin(),
                    m_curr_path.end(),
                    [&](const auto& comp) {
                        return boost::target(*loop_entry_it, graph)
                               == boost::source(comp, graph);
                    }
                );

                if (loop_exit_it != m_curr_path.end()) {
                    new_loops.emplace_back(loop_exit_it, m_curr_path.end());
                    new_loops.back().push_back(edge);
                    new_loops.back().insert(
                        new_loops.back().end(), loop_entry_it, loop.end()
                    );
                }
            }

            for (const auto& loop_2 : *m_loops) {
                auto loop_2_entry_it
                    = std::find_if(loop_2.begin(), loop_2.end(), [&](const auto& comp) {
                          return *loop_entry_it == boost::source(comp, graph);
                      });

                if (loop_2_entry_it == loop_2.end()) {
                    continue;
                }

                for (; loop_2_entry_it != loop.end(); ++loop_2_entry_it) {
                    // .....
                }
            }

            // If the current path is empty, the one situation we still need to handle
            // is if the loop we are considering exits into the root node of the DFS
            // traversal. If the current path is not empty, however, then we need only
            // consider the sources of the path edges as the target of the final edge in
            // the path is the source of the edge we are currently considering, and if
            // equal this would be a back edge.
            if (m_curr_path.empty()) {
                if (boost::target(loop.back(), graph) == m_root_vertex) {
                    // Create a new loop, starting at the forward edge that we are
                    // currently visiting (here an edge outbound from the root vertex.).
                    new_loops.emplace_back(std::vector<Edge>{ edge });
                    new_loops.back().insert(
                        new_loops.back().end(), loop_entry_it, loop.end()
                    );
                }
            } else {
                // Now, even though we link into a loop, this edge only contributes a
                // distinct loop if the loop also exits into the current path through
                // the DFS tree.
                auto loop_exit_it = std::find_if(
                    m_curr_path.begin(),
                    m_curr_path.end(),
                    [&](const auto& comp) {
                        return boost::target(loop.back(), graph)
                               == boost::source(comp, graph);
                    }
                );

                // Skip to next loop if the loop never exits into the path currently
                // traversed by DFS.
                if (loop_exit_it == m_curr_path.end()) continue;

                // Create a new loop, starting at the earliest vertex in the DFS tree
                // that constitutes part of the loop, moving down the tree and then
                // through the forward edge that we are currently visiting.
                new_loops.emplace_back(loop_exit_it, m_curr_path.end());
                new_loops.back().push_back(edge);
                new_loops.back().insert(
                    new_loops.back().end(), loop_entry_it, loop.end()
                );
            }
        }

        if (!new_loops.empty())
            m_loops->insert(m_loops->end(), new_loops.begin(), new_loops.end());
    }

    template <typename Graph>
    void halgo::LoopLocator<Graph>::start_vertex(Vertex vertex, const Graph&) {
        m_root_vertex = vertex;
    }

    template <typename Graph>
    void halgo::LoopLocator<Graph>::finish_vertex(Vertex, const Graph&) {
        if (!m_curr_path.empty()) m_curr_path.pop_back();
    }

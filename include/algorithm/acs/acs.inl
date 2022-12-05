template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::BasicACS() :
#if defined(HEMLOCK_DEBUG_ACS)
    m_get_2d_coord(nullptr),
    m_heatmap_frame_count(0),
    m_protoheatmap(nullptr),
    m_pheromone_heatmaps(nullptr),
    m_ant_count_heatmaps(nullptr),
#endif  // defined(HEMLOCK_DEBUG_ACS)
    m_max_iterations(100),
    m_break_on_path_change(0),
    m_break_on_iterations(2),
    m_exploitation_base(0.4f),
    m_exploitation_coeff(0.6f),
    m_exploitation_exponent(2.0f),
    m_do_mean_filtering(false),
    m_mean_filtering_order(0),
    m_mean_filtering_trigger(0.0f),
    m_local_increment(0.01f),
    m_local_evaporation(0.1f),
    m_global_increment(10.0f),
    m_global_evaporation(0.1f) {
    // Empty.
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_max_iterations(
    size_t max_iterations
) {
    m_max_iterations = max_iterations;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_break_on_path_change(f32 break_on_path_change) {
    m_break_on_path_change = break_on_path_change;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_break_on_iterations(size_t break_on_iterations) {
    m_break_on_iterations = break_on_iterations;

    return *this;
}

// template <typename VertexData, typename NextActionFinder, typename
// VertexChoiceStrategy> halgo::BasicACS<VertexData, NextActionFinder,
// VertexChoiceStrategy>& halgo::BasicACS<VertexData, NextActionFinder,
// VertexChoiceStrategy>::set_max_steps(size_t max_steps) {
//     m_max_steps = max_steps;
//
//     return *this;
// }

// template <typename VertexData, typename NextActionFinder, typename
// VertexChoiceStrategy> halgo::BasicACS<VertexData, NextActionFinder,
// VertexChoiceStrategy>& halgo::BasicACS<VertexData, NextActionFinder,
// VertexChoiceStrategy>::set_ant_count(size_t ant_count) {
//     m_ant_count = ant_count;
//
//     return *this;
// }

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_exploitation_base(f32 exploitation_base) {
    m_exploitation_base = exploitation_base;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_exploitation_coeff(f32 exploitation_coeff) {
    m_exploitation_coeff = exploitation_coeff;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_exploitation_exponent(f32 exploitation_exponent) {
    m_exploitation_exponent = exploitation_exponent;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_do_mean_filtering(bool do_mean_filtering) {
    m_do_mean_filtering = do_mean_filtering;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_mean_filtering_order(size_t mean_filtering_order) {
    m_mean_filtering_order = mean_filtering_order;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_mean_filtering_trigger(f32 mean_filtering_trigger) {
    m_mean_filtering_trigger = mean_filtering_trigger;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_local_increment(f32 local_increment) {
    m_local_increment = local_increment;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_local_evaporation(f32 local_evaporation) {
    m_local_evaporation = local_evaporation;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_global_increment(f32 global_increment) {
    m_global_increment = global_increment;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_global_evaporation(f32 global_evaporation) {
    m_global_evaporation = global_evaporation;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
template <size_t AntCount, size_t MaxSteps>
void halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::find_path(
    GraphMap<VertexData>& map,
    VertexData            source,
    VertexData            destination,
    VertexData*&          path,
    size_t&               path_length
) {
#if defined(HEMLOCK_DEBUG_ACS)
    initialise_heatmaps<MaxSteps>();
#endif  // defined(HEMLOCK_DEBUG_ACS)

    using _VertexDescriptor = VertexDescriptor<VertexData>;
    // using _EdgeDescriptor   = EdgeDescriptor<VertexData>;
    using _Ant      = Ant<_VertexDescriptor>;
    using _AntGroup = AntGroup<_VertexDescriptor, AntCount>;

    /******************\
     * Initialisation *
    \******************/

    _VertexDescriptor source_vertex      = map.coord_vertex_map[source];
    _VertexDescriptor destination_vertex = map.coord_vertex_map[destination];

    struct {
        _VertexDescriptor steps[MaxSteps + 1] = {};
        size_t            length              = std::numeric_limits<size_t>::max();
        bool              found               = false;
    } shortest_path;

    size_t last_shortest_path      = std::numeric_limits<size_t>::max();
    size_t satisfactory_change_its = 0;

    // TODO(Matthew): Heap allocation (paged?) at least for large number of ants & max
    // steps? Bucket of visited vertices for all ants. Vertices visited by each ant so
    // far on its path for an iteration.
    _VertexDescriptor visited_vertices[AntCount * (MaxSteps + 1)] = {};

    // The actual ants.
    const _Ant nil_ants[AntCount] = {};
    _Ant       ants[AntCount]     = {};

    for (size_t ant_idx = 0; ant_idx < AntCount; ++ant_idx) {
        ants[ant_idx].current_vertex = source_vertex;
    }

    // TODO(Matthew): Do we want to handle large AntCount? This isn't okay for that.
    // The ant groups used for calculating entropy.
    struct _AntGroups {
        _AntGroup groups[AntCount] = {};
        size_t    count            = 0;
    } ant_groups_new, ant_groups_old;

    for (auto edge : boost::make_iterator_range(boost::edges(map.graph))) {
        map.edge_weight_map[edge] += m_local_increment;
    }

    /*****************\
     * Do Iterations *
    \*****************/

    // Start off assuming minimal entropy, causes more exploration.
    f32 entropy = 0.0f;

    for (size_t iteration = 0; iteration < m_max_iterations; ++iteration) {
        std::memcpy(&ants[0], &nil_ants[0], AntCount * sizeof(_Ant));
        ant_groups_old = {};
        for (size_t ant_idx = 0; ant_idx < AntCount; ++ant_idx) {
            ants[ant_idx].current_vertex = source_vertex;
            ants[ant_idx].previous_vertices
                = &visited_vertices[ant_idx * (MaxSteps + 1)];
            ants[ant_idx].previous_vertices[0]     = source_vertex;
            ant_groups_old.groups[0].ants[ant_idx] = &ants[ant_idx];
        }
        ant_groups_old.groups[0].size = AntCount;
        ant_groups_old.count          = 1;

        // 2 * MaxSteps for return journey.
        //      TODO(Matthew): Do we just do one-way and apply pheromone
        //                     in separate subsequent pass?
        size_t ants_found_food = 0;
        for (size_t step = 0; step < /*2 **/ MaxSteps; ++step) {
            ant_groups_new       = {};
            ant_groups_new.count = ant_groups_old.count;

            size_t ant_group_cursors[AntCount] = {};

            for (size_t ant_idx = 0; ant_idx < AntCount; ++ant_idx) {
                _Ant& ant = ants[ant_idx];

                if (ant.found_food) {
                    // This group never changes for future steps.
                    _AntGroup& group         = ant_groups_new.groups[ant.group];
                    group.ants[group.size++] = &ant;

                    continue;
                }

                f32 exploitation_factor
                    = m_exploitation_base
                      + m_exploitation_coeff
                            * std::pow(entropy, m_exploitation_exponent);

                // TODO(Matthew): Reimplement back-stepping? Could be more performant
                // than
                //                simply doing more iterations to compensate for not
                //                doing this.

                auto [forward, next_vertex]
                    = VertexChoiceStrategy().template choose<NextActionFinder>(
                        ant, exploitation_factor, ant.current_vertex, map
                    );
                if (!forward) {
                    if (ant.steps_taken == 0) {
                        debug_printf(
                            "Could not step ant forward from initial vertex in "
                            "BasicACS::find_path."
                        );
                        return;
                    }

                    ant.steps_taken    -= 1;
                    ant.did_backstep   = true;
                    ant.current_vertex = ant.previous_vertices[ant.steps_taken];
                } else {
                    // Do local pheromone update.
                    auto [edge, _]
                        = boost::edge(ant.current_vertex, next_vertex, map.graph);

                    map.edge_weight_map[edge]
                        = (1.0f - m_local_evaporation) * map.edge_weight_map[edge]
                          + m_local_evaporation * m_local_increment;

                    // Update ant's vertex info.
                    ant.steps_taken                        += 1;
                    ant.previous_vertices[ant.steps_taken] = next_vertex;
                    ant.current_vertex                     = next_vertex;
                }

                bool need_new_group = false;
                bool changed_group  = false;

                // Increment cursor in ant's old group, use decremnted value
                // in subsequent loop.
                ant_group_cursors[ant.group] += 1;

                for (size_t cursor = 0; cursor < ant_group_cursors[ant.group] - 1;
                     ++cursor)
                {
                    _Ant& companion_ant
                        = *ant_groups_old.groups[ant.group].ants[cursor];

                    /**
                     * If we find an ant from this ant's previous path group who has
                     * moved to a new path group in this step, and both ants have
                     * moved to the same vertex, then we should put this current ant
                     * into the same path group. We have to be careful about how we
                     * performed this change, notes below detail the logic of how we
                     * ensure things don't go weird. We break when we get here as we
                     * have found the appropriate path group.
                     */
                    if (companion_ant.current_vertex == ant.current_vertex
                        && companion_ant.group != ant.group)
                    {
                        // Place ant in same group as companion ant who has taken
                        // same step.
                        auto& new_group = ant_groups_new.groups[companion_ant.group];
                        new_group.ants[new_group.size++] = &ant;

                        // Update ant's path group assignment.
                        ant.group = companion_ant.group;

                        // Ant no longer needs its own new path group.
                        need_new_group = false;
                        changed_group  = true;

                        // Break out of search for new path group.
                        break;
                    }

                    /**
                     * If this current ant has moved to a different vertex as an ant
                     * who was previously in the same path group and who has already
                     * taken their step, then this current ant no longer can belong to
                     * the same path group and so should be marked to receive one. We
                     * continue the loop however, in case another ant moved to the
                     * same vertex as this current ant.
                     */
                    if (companion_ant.current_vertex != ant.current_vertex
                        && companion_ant.group == ant.group)
                    {
                        need_new_group = true;
                        changed_group  = false;
                    }
                }

                /**
                 * If this current ant is flagged as needing a new path group,
                 * then make it one and put it in it.
                 *      While we don't have to worry about how it gets into the new
                 *      path group, we still have to be careful of how it leaves its
                 *      old one.
                 */
                if (need_new_group) {
                    // Add new group on at end of list of groups, to avoid issues with
                    // groups that already exist.
                    auto& new_group = ant_groups_new.groups[ant_groups_new.count];
                    new_group.ants[new_group.size++] = &ant;

                    ant.group = ant_groups_new.count;

                    ant_groups_new.count += 1;
                } else if (!changed_group) {
                    // Place ant in group is was in on last step, it hasn't moved.
                    auto& new_group                  = ant_groups_new.groups[ant.group];
                    new_group.ants[new_group.size++] = &ant;
                }

                if (next_vertex == destination_vertex) {
                    ant.found_food  = true;
                    ants_found_food += 1;

                    if (ant.steps_taken + 1 < shortest_path.length) {
                        shortest_path.length = ant.steps_taken + 1;
                        shortest_path.found  = true;
                        std::memcpy(
                            &shortest_path.steps[0],
                            ant.previous_vertices,
                            sizeof(_VertexDescriptor) * (ant.steps_taken + 1)
                        );
                    }
                }
            }

            // TODO(Matthew): remove by referring to groups by reference only.
            std::memcpy(&ant_groups_old, &ant_groups_new, sizeof(_AntGroups));

#if defined(HEMLOCK_DEBUG_ACS)
            create_pheromone_heatmap_frame<MaxSteps>(
                &m_pheromone_heatmaps[m_heatmap_frame_count], map
            );

            create_ant_count_heatmap_frame<AntCount, MaxSteps>(
                &m_ant_count_heatmaps[m_heatmap_frame_count], map, &ants[0]
            );

            m_heatmap_frame_count += 1;
#endif  // defined(HEMLOCK_DEBUG_ACS)

            if (ants_found_food == AntCount) break;
        }

        // Calculate entropy of iteration.
        entropy = 0.0f;
        for (size_t ant_group_idx = 0; ant_group_idx < ant_groups_new.count;
             ++ant_group_idx)
        {
            f32 popularity = static_cast<f32>(ant_groups_new.groups[ant_group_idx].size)
                             / static_cast<f32>(AntCount);
            entropy += popularity * log(popularity);
        }
        entropy /= log(1.0f / static_cast<f32>(AntCount));

        // Global pheromone update.

        for (auto edge : boost::make_iterator_range(boost::edges(map.graph))) {
            map.edge_weight_map[edge] *= (1.0f - m_global_evaporation);
        }

        if (shortest_path.found) {
            for (size_t step_idx = 0; step_idx < shortest_path.length - 1; ++step_idx) {
                auto [edge, _] = boost::edge(
                    shortest_path.steps[step_idx],
                    shortest_path.steps[step_idx + 1],
                    map.graph
                );

                map.edge_weight_map[edge]
                    += m_global_evaporation
                       * (m_global_increment / static_cast<f32>(shortest_path.length));
            }
        }

        if (shortest_path.found
            && glm::max(last_shortest_path, shortest_path.length)
                       - glm::min(last_shortest_path, shortest_path.length)
                   <= m_break_on_path_change)
        {
            satisfactory_change_its += 1;

            if (satisfactory_change_its >= m_break_on_iterations) break;
        } else {
            satisfactory_change_its = 0;
        }

        last_shortest_path = shortest_path.length;
    }

    // TODO(Matthew): better...
    if (shortest_path.found) {
        path_length = shortest_path.length;
        path        = new VertexData[path_length];

        for (size_t idx = 0; idx < path_length; ++idx) {
            path[idx] = map.vertex_coord_map[shortest_path.steps[idx]];
        }
    }
}

#if defined(HEMLOCK_DEBUG_ACS)
template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_protoheatmap(
    heatmap_t* protoheatmap
) {
    m_protoheatmap = protoheatmap;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    set_vertex_to_2d_coord(VertexDataTo2DCoord<VertexData> get_2d_coord) {
    m_get_2d_coord = get_2d_coord;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
template <size_t MaxSteps>
void halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    initialise_heatmaps() {
    if (m_protoheatmap == nullptr) return;

    // Allocate all heatmap buffers and the heatmaps themselves as blocks and divy out.
    f32* buffers = new f32
        [MaxSteps * m_max_iterations * 2 * m_protoheatmap->w * m_protoheatmap->h]();

    heatmap_t* heatmaps = new heatmap_t[MaxSteps * m_max_iterations * 2]();

    for (size_t heatmap_idx = 0; heatmap_idx < MaxSteps * m_max_iterations * 2;
         heatmap_idx++)
    {
        heatmaps[heatmap_idx].w = m_protoheatmap->w;
        heatmaps[heatmap_idx].h = m_protoheatmap->h;
        heatmaps[heatmap_idx].buf
            = &buffers[heatmap_idx * m_protoheatmap->w * m_protoheatmap->h];
    }

    m_pheromone_heatmaps = &heatmaps[0];
    m_ant_count_heatmaps = &heatmaps[MaxSteps * m_max_iterations];
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
void halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    dispose_heatmaps() {
    if (m_pheromone_heatmaps == nullptr) return;

    delete[] m_pheromone_heatmaps[0].buf;
    delete[] m_pheromone_heatmaps;

    m_pheromone_heatmaps = nullptr;
    m_ant_count_heatmaps = nullptr;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
template <size_t MaxSteps>
void halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    create_pheromone_heatmap_frame(heatmap_t* heatmap, GraphMap<VertexData>& map) {
    if (m_protoheatmap == nullptr) return;

    // For each vertex in graph, sum up pheromone on edges leading into it and add
    // a point weighted by this to the heatmap.
    for (auto vertex : boost::make_iterator_range(boost::vertices(map.graph))) {
        f32 net_pheromone_into_vertex = 0.0f;
        for (auto edge : boost::make_iterator_range(boost::in_edges(vertex, map.graph)))
            net_pheromone_into_vertex += map.edge_weight_map[edge];

        auto [x, y] = m_get_2d_coord(map.vertex_coord_map[vertex]);

        heatmap_add_weighted_point(heatmap, x, y, net_pheromone_into_vertex);
    }

    // Apply protoheatmap to this heatmap frame factoring in strongest pheromone signal.
    size_t pixel_count = m_protoheatmap->w * m_protoheatmap->h;
    for (size_t pixel_idx = 0; pixel_idx < pixel_count; ++pixel_idx) {
        heatmap->buf[pixel_idx] += m_protoheatmap->buf[pixel_idx] * heatmap->max;
    }
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
template <size_t AntCount, size_t MaxSteps>
void halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::
    create_ant_count_heatmap_frame(
        heatmap_t*                         heatmap,
        GraphMap<VertexData>&              map,
        Ant<VertexDescriptor<VertexData>>* ants
    ) {
    if (m_protoheatmap == nullptr) return;

    // For each ant add point to heatmap for its current location on the graph.
    for (size_t ant_idx = 0; ant_idx < AntCount; ++ant_idx) {
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
#endif  // defined(HEMLOCK_DEBUG_ACS)

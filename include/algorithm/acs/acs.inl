template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::BasicACS() :
    m_max_iterations(100),
    m_break_on_path_change(0.0f),
    m_break_on_iterations(2),
    m_exploitation_base(0.6f),
    m_exploitation_coeff(0.4f),
    m_exploitation_exponent(2.0f),
    m_do_mean_filtering(false),
    m_mean_filtering_order(0),
    m_mean_filtering_trigger(0.0f),
    m_local_increment(0.01f),
    m_local_evaporation(0.1f),
    m_global_increment(1.0f),
    m_global_evaporation(0.1f)
{
    // Empty.
}


template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_max_iterations(size_t max_iterations) {
    m_max_iterations = max_iterations;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_break_on_path_change(f32 break_on_path_change) {
    m_break_on_path_change = break_on_path_change;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_break_on_iterations(size_t break_on_iterations) {
    m_break_on_iterations = break_on_iterations;

    return *this;
}

// template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
// halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
// halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_max_steps(size_t max_steps) {
//     m_max_steps = max_steps;
//
//     return *this;
// }

// template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
// halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
// halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_ant_count(size_t ant_count) {
//     m_ant_count = ant_count;
//
//     return *this;
// }

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_exploitation_base(f32 exploitation_base) {
    m_exploitation_base = exploitation_base;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_exploitation_coeff(f32 exploitation_coeff) {
    m_exploitation_coeff = exploitation_coeff;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_exploitation_exponent(f32 exploitation_exponent) {
    m_exploitation_exponent = exploitation_exponent;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_do_mean_filtering(bool do_mean_filtering) {
    m_do_mean_filtering = do_mean_filtering;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_mean_filtering_order(size_t mean_filtering_order) {
    m_mean_filtering_order = mean_filtering_order;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_mean_filtering_trigger(f32 mean_filtering_trigger) {
    m_mean_filtering_trigger = mean_filtering_trigger;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_local_increment(f32 local_increment) {
    m_local_increment = local_increment;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_local_evaporation(f32 local_evaporation) {
    m_local_evaporation = local_evaporation;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_global_increment(f32 global_increment) {
    m_global_increment = global_increment;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>&
halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>::set_global_evaporation(f32 global_evaporation) {
    m_global_evaporation = global_evaporation;

    return *this;
}

template <typename VertexData, typename NextActionFinder, typename VertexChoiceStrategy>
template <size_t AntCount, size_t MaxSteps>
void halgo::BasicACS<VertexData, NextActionFinder, VertexChoiceStrategy>
        ::find_path(GraphMap<VertexData>& map, VertexData source, VertexData destination, VertexData*& path, size_t& path_length) {
    using _VertexDescriptor = VertexDescriptor<VertexData>;
    // using _EdgeDescriptor   = EdgeDescriptor<VertexData>;
    using _Ant              = Ant<_VertexDescriptor>;
    using _AntGroup         = AntGroup<_VertexDescriptor, AntCount>;

    /******************\
     * Initialisation *
    \******************/

    _VertexDescriptor source_vertex         = map.coord_vertex_map[source];
    _VertexDescriptor destination_vertex    = map.coord_vertex_map[destination];

    struct {
        _VertexDescriptor steps[MaxSteps]   = {};
        size_t            length            = std::numeric_limits<size_t>::max();
        bool              found             = false;
    } shortest_path;
    size_t last_shortest_path = std::numeric_limits<size_t>::max();
    size_t satisfactory_change_its = 0;

    // TODO(Matthew): Heap allocation (paged?) at least for large number of ants & max steps?
    // Bucket of visited vertices for all ants.
    // Vertices visited by each ant so far on its path for an iteration.
    _VertexDescriptor visited_vertices[AntCount * MaxSteps] = {};

    // The actual ants.
    const _Ant nil_ants[AntCount] = {};
    _Ant ants[AntCount] = {};

    for (size_t ant_idx = 0; ant_idx < AntCount; ++ant_idx) {
        ants[ant_idx].current_vertex = source_vertex;
    }

    // TODO(Matthew): Do we want to handle large AntCount? This isn't okay for that.
    // The ant groups used for calculating entropy.
    struct _AntGroups {
        _AntGroup groups[AntCount]  = {};
        size_t    count             = 0;
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
        ant_groups_old  = {};
        for (size_t ant_idx = 0; ant_idx < AntCount; ++ant_idx) {
            ants[ant_idx].current_vertex = source_vertex;
            ants[ant_idx].previous_vertices = &visited_vertices[ant_idx * MaxSteps];
            ant_groups_old.groups[0].ants[ant_idx] = &ants[ant_idx];
        }
        ant_groups_old.groups[0].size = AntCount;
        ant_groups_old.count = 1;

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

                // TODO(Matthew): does this have consequences for entropy calculation?
                if (!ant.alive || ant.found_food) continue;

                f32 exploitation_factor = m_exploitation_base + m_exploitation_coeff * std::pow((1.0f - entropy), m_exploitation_exponent);

                // TODO(Matthew): Reimplement back-stepping? Could be more performant than
                //                simply doing more iterations to compensate for not doing this.

                auto [found, next_vertex] = VertexChoiceStrategy().template choose<NextActionFinder>(ant, exploitation_factor, ant.current_vertex, map);
                if (!found) {
                    // TODO(Matthew): does this have consequences for entropy calculation?
                    ant.alive = false;
                    // continue;
                } else {
                    // Update ant's vertex info.
                    ant.previous_vertices[step] = next_vertex;
                    ant.current_vertex          = next_vertex;
                }

                bool need_new_group = false;
                bool changed_group  = false;

                // Increment cursor in ant's old group, use decremnted value
                // in subsequent loop.
                ant_group_cursors[ant.group] += 1;

                for (size_t cursor = 0; cursor < ant_group_cursors[ant.group] - 1; ++cursor) {
                    _Ant& companion_ant = *ant_groups_old.groups[ant.group].ants[cursor];

                    // Don't consider putting in a group with a dead ant.
                    if (!companion_ant.alive) continue;

                    /**
                     * If we find an ant from this ant's previous path group who has moved to a new path group
                     * in this step, and both ants have moved to the same vertex, then we should put this current
                     * ant into the same path group.
                     *      We have to be careful about how we performed this change, notes below detail the logic
                     *      of how we ensure things don't go weird.
                     *      We break when we get here as we have found the appropriate path group.
                     */
                    if (   companion_ant.current_vertex == ant.current_vertex
                                 && companion_ant.group != ant.group      ) {
                        // Place ant in same group as companion ant who has taken
                        // same step.
                        auto& new_group = ant_groups_new.groups[companion_ant.group];
                        new_group.ants[new_group.size++] = &ant;

                        // Update ant's path group assignment.
                        ant.group = companion_ant.group;

                        // Ant no longer needs its own new path group.
                        need_new_group  = false;
                        changed_group   = true;

                        // Break out of search for new path group.
                        break;
                    }

                    /**
                     * If this current ant has moved to a different vertex as an ant who was previously
                     * in the same path group and who has already taken their step, then this current
                     * ant no longer can belong to the same path group and so should be marked to receive
                     * one.
                     *      We continue the loop however, in case another ant moved to the same vertex
                     *      as this current ant.
                     */
                    if (   companion_ant.current_vertex != ant.current_vertex
                            && companion_ant.group == ant.group      ) {
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
                    auto& new_group = ant_groups_new.groups[ant.group];
                    new_group.ants[new_group.size++] = &ant;
                }

                if (next_vertex == destination_vertex) {
                    ant.found_food  = true;
                    ant.steps_taken = step + 1;

                    ants_found_food += 1;

                    if (ant.steps_taken < shortest_path.length) {
                        shortest_path.length = ant.steps_taken;
                        shortest_path.found  = true;
                        std::memcpy(&shortest_path.steps[0], ant.previous_vertices, sizeof(_VertexDescriptor) * ant.steps_taken);
                    }
                }
            }

            // TODO(Matthew): remove by referring to groups by reference only.
            std::memcpy(&ant_groups_old, &ant_groups_new, sizeof(_AntGroups));

            if (ants_found_food == AntCount) break;
        }

        for (size_t ant_group_idx = 0; ant_group_idx < ant_groups_new.count; ++ant_group_idx) {
            _VertexDescriptor step_start, step_end;
            step_start = source_vertex;

            for (size_t step_idx = 0; step_idx < ant_groups_new.groups[ant_group_idx].ants[0]->steps_taken; ++step_idx) {
                step_end = ant_groups_new.groups[ant_group_idx].ants[0]->previous_vertices[step_idx];

                // TODO(Matthew): edge may not exist.
                auto [edge, _] = boost::edge(step_start, step_end, map.graph);

                // TODO(Matthew): Do we need to apply evaporation per ant? Also, do we
                //                need to apply evaporation to every edge or just those
                //                that the ants walk over?
                map.edge_weight_map[edge] =
                            (1.0f - m_local_evaporation) * map.edge_weight_map[edge]
                          +         m_local_evaporation  * m_local_increment * static_cast<f32>(ant_groups_new.groups[ant_group_idx].size);

                step_start = step_end;
            }
        }

        // Calculate entropy of iteration.
        entropy = 0.0f;
        for (size_t ant_group_idx = 0; ant_group_idx < ant_groups_new.count; ++ant_group_idx) {
            f32 popularity = static_cast<f32>(ant_groups_new.groups[ant_group_idx].size) / static_cast<f32>(AntCount);
            entropy += popularity * log(popularity);
        }
        entropy /= log(1.0f / static_cast<f32>(AntCount));

        // Global pheromone update.

        for (auto edge : boost::make_iterator_range(boost::edges(map.graph))) {
                map.edge_weight_map[edge] *= (1.0f - m_global_evaporation);
        }

        if (shortest_path.found) {
            for (size_t step_idx = 0; step_idx < shortest_path.length - 1; ++step_idx) {
                // TODO(Matthew): edge may not exist.
                auto [edge, _] = boost::edge(shortest_path.steps[step_idx], shortest_path.steps[step_idx + 1], map.graph);

                map.edge_weight_map[edge] +=
                    m_global_evaporation
                        * (m_global_increment / static_cast<f32>(shortest_path.length));
            }
        }

        if (
            shortest_path.found
                && glm::max(last_shortest_path, shortest_path.length)
                    - glm::min(last_shortest_path, shortest_path.length)
                        <= m_break_on_path_change
        ) {
            satisfactory_change_its += 1;

            if (satisfactory_change_its >= m_break_on_iterations) break;
        } else {
            satisfactory_change_its = 0;
        }
    }

    // TODO(Matthew): better...
    if (shortest_path.found) {
        path_length = shortest_path.length;
        path = new VertexData[path_length];

        for (size_t idx = 0; idx < path_length; ++idx) {
            path[idx] = map.vertex_coord_map[shortest_path.steps[idx]];
        }
    }
}

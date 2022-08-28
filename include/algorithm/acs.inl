template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_max_iterations(size_t max_iterations) {
    m_max_iterations = max_iterations;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_break_on_path_change(f32 break_on_path_change) {
    m_break_on_path_change = break_on_path_change;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_break_on_iterations(size_t break_on_iterations) {
    m_break_on_iterations = break_on_iterations;

    return *this;
}

// template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
// halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
// halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_max_steps(size_t max_steps) {
//     m_max_steps = max_steps;
//
//     return *this;
// }

// template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
// halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
// halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_ant_count(size_t ant_count) {
//     m_ant_count = ant_count;
//
//     return *this;
// }

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_exploitation_base(f32 exploitation_base) {
    m_exploitation_base = exploitation_base;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_exploitation_coeff(f32 exploitation_coeff) {
    m_exploitation_coeff = exploitation_coeff;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_exploitation_exponent(f32 exploitation_exponent) {
    m_exploitation_exponent = exploitation_exponent;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_do_mean_filtering(bool do_mean_filtering) {
    m_do_mean_filtering = do_mean_filtering;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_mean_filtering_order(size_t mean_filtering_order) {
    m_mean_filtering_order = mean_filtering_order;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_mean_filtering_trigger(f32 mean_filtering_trigger) {
    m_mean_filtering_trigger = mean_filtering_trigger;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_local_increment(f32 local_increment) {
    m_local_increment = local_increment;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_local_evaporation(f32 local_evaporation) {
    m_local_evaporation = local_evaporation;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_global_increment(f32 global_increment) {
    m_global_increment = global_increment;

    return *this;
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>&
halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>::set_global_evaporation(f32 global_evaporation) {
    m_global_evaporation = global_evaporation;

    return *this;
}

namespace hemlock {
    namespace algorithm {
        namespace impl {
            template <size_t AntCount, typename VertexCoordType, typename VertexDescriptor>
            void reset_ants(Ant<VertexCoordType, VertexDescriptor>* ants) {
                for (size_t i = 0; i < AntCount; ++i) {
                    ants[i].has_food = false;
                }
            }
        }
    }
}

template <typename VertexCoordType, typename NextStepGenerator, typename VertexChoiceStrategy>
template <size_t AntCount, size_t MaxSteps>
void halgo::BasicACS<VertexCoordType, NextStepGenerator, VertexChoiceStrategy>
        ::find_path(ACSGraphMap<VertexCoordType>& map, VertexCoordType source, VertexCoordType destination, ui64 seed) {
    using _VertexDescriptor = details::VertexDescriptor<VertexCoordType>;
    using _EdgeDescriptor   = details::EdgeDescriptor<VertexCoordType>;
    using _Ant              = Ant<VertexCoordType, _VertexDescriptor>;
    using _AntGroup         = AntGroup<VertexCoordType, _VertexDescriptor, AntCount>;

    /******************\
     * Initialisation *
    \******************/

    _VertexDescriptor source_vertex         = map.coord_vertex_map[source];
    _VertexDescriptor destination_vertex    = map.coord_vertex_map[destination];

    struct {
        _VertexDescriptor steps[MaxSteps];
        size_t            length;
    } shortest_path;

    // TODO(Matthew): Heap allocation (paged?) at least for large number of ants & max steps?
    // Bucket of visited vertices for all ants.
    // Vertices visited by each ant so far on its path for an iteration.
    _VertexDescriptor visited_vertices[AntCount * MaxSteps] = {};

    // The actual ants.
    _Ant ants[AntCount];

    // TODO(Matthew): Do we want to handle large AntCount? This isn't okay for that.
    // The ant groups used for calculating entropy.
    _AntGroup ant_groups_new[AntCount], ant_groups_old[AntCount];
    // The number of ant groups active.
    size_t ant_group_count = 0;

    for (size_t ant_idx = 0; ant_idx < AntCount; ++ant_idx) {
        ants[ant_idx].previous_vertices = &visited_vertices[ant_idx * MaxSteps];
    }

    for (auto edge : boost::make_iterator_range(boost::edges(map.graph))) {
        map.edge_weight_map[edge] += m_local_increment;
    }

    std::default_random_engine            generator;
    std::uniform_real_distribution<float> distrib(0.0f, 1.0f);

    generator.seed(seed);

    /*****************\
     * Do Iterations *
    \*****************/

    // Start off assuming maximal entropy, causes
    f32 entropy = 1.0f;

    for (size_t iteration = 0; iteration < m_max_iterations; ++iteration) {
        ants            = {};
        ant_groups_old  = {};
        ant_groups_new  = {};
        for (size_t ant_idx = 0; ant_idx < AntCount; ++ant_idx) {
            ants[ant_idx].current_vertex = source_vertex;
            ant_groups_old[0].ants[ant_idx] = &ants[ant_idx];
            ant_groups_new[0].ants[ant_idx] = &ants[ant_idx];
        }
        ant_groups_old[0].size = AntCount;
        ant_groups_new[0].size = AntCount;
        ant_group_count = 1;

        // 2 * MaxSteps for return journey.
        //      TODO(Matthew): Do we just do one-way and apply pheromone
        //                     in separate subsequent pass?
        size_t ants_found_food = 0;
        for (size_t step = 0; step < /*2 **/ MaxSteps; ++step) {
            size_t ant_group_cursors[AntCount] = {};

            for (size_t ant_idx = 0; ant_idx < AntCount; ++ant_idx) {
                _Ant& ant = ants[ant_idx];

                if (ant.found_food) continue;

                f32 exploitation_factor = m_exploitation_base + m_exploitation_coeff * std::pow(entropy, m_exploitation_exponent);

                // TODO(Matthew): Reimplement back-stepping? Could be more performant than
                //                simply doing more iterations to compensate for not doing this.

                _VertexDescriptor next_vertex = ??;
                

                // TODO(Matthew): Think about how to do this better, where we swap ants about
                //                we are affecting order and this means the assumption that
                //                each ant group has ants ordered by their index in the ants
                //                array is false.
                //                  Change data structure as the current actually requires
                //                  a "shuffle" to keep things matching assumptions.

                // bool need_new_group = false;
                // bool joined_existing_group = false;

                // for (size_t cursor = 0; cursor < ant_group_cursors[ant.group]; ++cursor) {
                //     Ant& companion_ant = *ant_groups_old[ant.group][cursor];

                //     /**
                //      * If we find an ant from this ant's previous path group who has moved to a new path group
                //      * in this step, and both ants have moved to the same vertex, then we should put this current
                //      * ant into the same path group.
                //      *      We have to be careful about how we performed this change, notes below detail the logic
                //      *      of how we ensure things don't go weird.
                //      *      We break when we get here as we have found the appropriate path group.
                //      */
                //     if (   companion_ant.current_vertex == ant.current_vertex
                //                  && companion_ant.group != ant.group      ) {
                //         // Remove ant from previous path group.
                //         //   It should be fine to just remove ant from previous path group like this as
                //         //   it will be replaced in its position by an ant beyond the cursor for that path group.
                //         _Ant* tmp = ant_groups_new[ant.group].ants[ant_group_cursors[ant.group]];
                //         ant_groups_new[ant.group].ants[ant_group_cursors[ant.group] - 1];

                //         // Update ant's path group assignment.
                //         ant.group = companion_ant.group;
                //         // Make sure to put ant in path group at current point of that path group's cursor.
                //         auto& group = ant_path_tracker_new[companion_ant.group];
                //         group.push_back(group[group_curors[ant.group]]);
                //         group[group_curors[ant.group]] = &ant;
                //         // Ant no longer needs its own new path group.
                //         need_new_group   = false;
                //         joined_existing_group = true;
                //         // Break out of search for new path group.
                //         break;
                //     }

                //     /**
                //      * If this current ant has moved to a different vertex as an ant who was previously
                //      * in the same path group and who has already taken their step, then this current
                //      * ant no longer can belong to the same path group and so should be marked to receive
                //      * one.
                //      *      We continue the loop however, in case another ant moved to the same vertex
                //      *      as this current ant.
                //      */
                //     if (   companion_ant.current_vertex != ant.current_vertex
                //             && companion_ant.group == ant.group      ) {
                //         need_new_group = true;
                //     }
                // }

                // /**
                //  * If this current ant is flagged as needing a new path group,
                //  * then make it one and put it in it.
                //  *      While we don't have to worry about how it gets into the new
                //  *      path group, we still have to be careful of how it leaves its
                //  *      old one.
                //  */
                // if (need_new_group) {
                //     size_t new_group = ant_group_count;

                //     // Remove ant from previous path group.
                //     //   It should be fine to just remove ant from previous path group like this as
                //     //   it will be replaced in its position by an ant beyond the cursor for that path group.
                //     std::remove(ant_path_tracker_new[ant.group].begin(), ant_path_tracker_new[ant.path_group].end(), &ant);
                //     ant_path_tracker_new[ant.path_group].erase(ant_path_tracker_new[ant.path_group].end() - 1);

                //     ant_path_tracker_new[new_path_group] = Ants(1, &ant);
                //     path_group_curors[new_path_group] = 0;
                //     ant.path_group = new_path_group;
                // } else if (!joined_existing_group) {
                //     path_group_curors[ant.path_group] += 1;
                // }

                ant.previous_vertices[step] = next_vertex;
                ant.current_vertex          = next_vertex;

                if (next_vertex == destination_vertex) {
                    ant.found_food  = true;
                    ant.steps_taken = step + 1;

                    ants_found_food += 1;
                }
            }

            if (ants_found_food == AntCount) break;
        }

        std::memcpy(&ant_groups_old, &ant_groups_new, AntCount * sizeof(_AntGroup));

        for (size_t ant_group_idx = 0; ant_group_idx < ant_group_count; ++ant_group_idx) {
            _VertexDescriptor step_start, step_end;
            step_start = source_vertex;

            for (size_t step_idx = 0; step_idx < ant_groups_new[ant_group_idx].ants[0].steps_taken; ++step_idx) {
                step_end = ant_groups_new[ant_group_idx].ants[0].previous_vertices[step_idx];

                auto [edge, _] = boost::edge(step_start, step_end, map.graph);

                map.edge_weight_map[edge] =
                            (1.0f - m_local_evaporation) * map.edge_weight_map[edge]
                          +         m_local_evaporation  * m_local_increment;

                step_start = step_end;
            }
        }

        // Calculate entropy of iteration.
        entropy = 0.0f;
        for (size_t ant_group_idx = 0; ant_group_idx < ant_group_count; ++ant_group_idx) {
            f32 popularity = static_cast<f32>(ant_groups_new[ant_group_idx].size) / static_cast<f32>(AntCount);
            entropy += popularity * log(popularity);
        }
        entropy /= log(1.0f / static_cast<f32>(AntCount));

        // Global pheromone update.
        for (auto edge : boost::make_iterator_range(boost::edges(map.graph))) {
                map.edge_weight_map[edge] *= (1.0f - m_global_evaporation);

                /**
                 * If edge is in shortest path, we must apply the inverse
                 * length component of the global updating rule.
                 */
                if (map.edge_in_path_map[edge]) {
                    map.edge_weight_map[edge] +=
                        m_global_evaporation
                            * (m_global_increment / static_cast<f32>(shortest_path.length));
                }
        }

        // TODO(Matthew): Do some early break checking using m_break_on_*.
    }
}

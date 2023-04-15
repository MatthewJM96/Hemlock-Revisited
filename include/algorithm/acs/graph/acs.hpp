#ifndef __hemlock_algorithm_acs_graph_acs_hpp
#define __hemlock_algorithm_acs_graph_acs_hpp

#include "../state.hpp"

namespace hemlock {
    namespace algorithm {
        namespace debug {
            template <typename Node, bool IsWeighted>
            class ACSHeatmap2D;
        }
        namespace deb = debug;

        namespace GraphACS {
            template <
                typename Node,
                bool                                    IsWeighted,
                ACSConfig                               Config,
                ACSDistanceCalculator<Node, IsWeighted> DistanceCalculator
                = NullACSDistanceCalculator<Node, IsWeighted>>
            void find_path(
                GraphMap<Node, IsWeighted>&          map,
                Node                                 source,
                Node                                 destination,
                Node*&                               path,
                size_t&                              path_length,
                deb::ACSHeatmap2D<Node, IsWeighted>* debugger = nullptr
            ) {
                if constexpr (Config.debug.on) {
                    if (debugger)
                        debugger->initialise_heatmaps(
                            Config.max_steps, Config.max_iterations
                        );
                }

                using _EdgeDescriptor   = EdgeDescriptor<Node, IsWeighted>;
                using _VertexDescriptor = VertexDescriptor<Node, IsWeighted>;
                using _Ant              = Ant<_VertexDescriptor>;
                using _AntGroup         = AntGroup<_VertexDescriptor, Config.ant_count>;

                const auto calculate_score = [&](Node candidate, _EdgeDescriptor edge) {
                    // TODO(Matthew): Fully expose this (at least optionally), likewise
                    // make even providing distance calc
                    //                and modifying score with it optional.

                    const DistanceCalculator calc_distance{};

                    f32 score = map.pheromone_map[edge];

                    score
                        /= (1.0f + calc_distance(map, source, destination, candidate));

                    return score;
                };

                /******************\
                 * Initialisation *
                \******************/

                _VertexDescriptor source_vertex = map.coord_vertex_map[source];
                _VertexDescriptor destination_vertex
                    = map.coord_vertex_map[destination];

                struct {
                    _VertexDescriptor steps[Config.max_steps + 1] = {};
                    size_t            length = std::numeric_limits<size_t>::max();
                    bool              found  = false;
                } shortest_path;

                size_t last_shortest_path      = std::numeric_limits<size_t>::max();
                size_t satisfactory_change_its = 0;

                // TODO(Matthew): Heap allocation (paged?) at least for large number of
                // ants & max steps? Bucket of visited vertices for all ants. Vertices
                // visited by each ant so far on its path for an iteration.
                _VertexDescriptor
                    visited_vertices[Config.ant_count * (Config.max_steps + 1)]
                    = {};

                // The actual ants.
                const _Ant nil_ants[Config.ant_count] = {};
                _Ant       ants[Config.ant_count]     = {};

                for (size_t ant_idx = 0; ant_idx < Config.ant_count; ++ant_idx) {
                    ants[ant_idx].current_vertex = source_vertex;
                }

                // TODO(Matthew): Do we want to handle large Config.ant_count? This
                // isn't okay for that. The ant groups used for calculating entropy.
                struct _AntGroups {
                    _AntGroup groups[Config.ant_count] = {};
                    size_t    count                    = 0;
                } ant_groups_new, ant_groups_old;

                for (auto edge : boost::make_iterator_range(boost::edges(map.graph))) {
                    map.pheromone_map[edge] += Config.local.increment;
                }

                /*****************\
                 * Do Iterations *
                \*****************/

                // Start off assuming minimal entropy, causes more exploration.
                f32 entropy = 0.0f;

                for (size_t iteration = 0; iteration < Config.max_iterations;
                     ++iteration)
                {
                    std::memcpy(
                        &ants[0], &nil_ants[0], Config.ant_count * sizeof(_Ant)
                    );
                    ant_groups_old = {};
                    for (size_t ant_idx = 0; ant_idx < Config.ant_count; ++ant_idx) {
                        ants[ant_idx].current_vertex = source_vertex;
                        ants[ant_idx].previous_vertices
                            = &visited_vertices[ant_idx * (Config.max_steps + 1)];
                        ants[ant_idx].previous_vertices[0]     = source_vertex;
                        ant_groups_old.groups[0].ants[ant_idx] = &ants[ant_idx];
                    }
                    ant_groups_old.groups[0].size = Config.ant_count;
                    ant_groups_old.count          = 1;

                    size_t ants_found_food = 0;
                    for (size_t step = 0; step < /*2 **/ Config.max_steps; ++step) {
                        // TODO(Matthew): can we lazily reset ants?
                        ant_groups_new       = {};
                        ant_groups_new.count = ant_groups_old.count;

                        size_t ant_group_cursors[Config.ant_count] = {};

                        for (size_t ant_idx = 0; ant_idx < Config.ant_count; ++ant_idx)
                        {
                            _Ant& ant = ants[ant_idx];

                            if (ant.found_food) {
                                // This group never changes for future steps.
                                _AntGroup& group = ant_groups_new.groups[ant.group];
                                group.ants[group.size++] = &ant;

                                continue;
                            }

                            f32 exploitation_factor
                                = Config.exploitation.base
                                  + Config.exploitation.coeff
                                        * std::pow(entropy, Config.exploitation.exp);

                            auto choose = [&]() -> std::tuple<bool, _VertexDescriptor> {
                                auto edges = boost::make_iterator_range(
                                    boost::out_edges(ant.current_vertex, map.graph)
                                );

                                static std::random_device rand_dev;
#if defined(DEBUG)
                                static std::mt19937 generator(1337);
#else
                                static std::mt19937 generator(rand_dev());
#endif  // defined(DEBUG)
                                static std::uniform_real_distribution<f32> distribution(
                                    0.0f, 1.0f
                                );

                                f32 rand_val = distribution(generator);
                                if (rand_val < exploitation_factor) {
                                    bool              found_best     = false;
                                    _VertexDescriptor best_candidate = 0;
                                    f32 best_score = std::numeric_limits<f32>::lowest();
                                    for (auto edge : edges) {
                                        _VertexDescriptor candidate_vertex
                                            = boost::target(edge, map.graph);

                                        /**
                                         * If ant has just visited the candidate vertex,
                                         * then reject it as a candidate. "Just visited"
                                         * is the step earlier in all cases, but if the
                                         * ant has just stepped backwards this also
                                         * includes the next in previous_vertices from
                                         * the current step.
                                         */
                                        if (ant.steps_taken > 0
                                            && ant.previous_vertices
                                                       [ant.steps_taken - 1]
                                                   == candidate_vertex)
                                            continue;

                                        if (ant.did_backstep && ant.steps_taken > 0
                                            && ant.previous_vertices
                                                       [ant.steps_taken + 1]
                                                   == candidate_vertex)
                                            continue;

                                        f32 score = calculate_score(
                                            map.vertex_coord_map[candidate_vertex], edge
                                        );

                                        if (score > best_score) {
                                            found_best     = true;
                                            best_score     = score;
                                            best_candidate = candidate_vertex;
                                        }
                                    }

                                    if (found_best) {
                                        return { true, best_candidate };
                                    } else {
                                        return { false, {} };
                                    }
                                } else {
                                    f32 total_score = 0.0f;
                                    for (auto edge : edges) {
                                        _VertexDescriptor candidate_vertex
                                            = boost::target(edge, map.graph);

                                        /**
                                         * If ant has just visited the candidate vertex,
                                         * then reject it as a candidate. "Just visited"
                                         * is the step earlier in all cases, but if the
                                         * ant has just stepped backwards this also
                                         * includes the next in previous_vertices from
                                         * the current step.
                                         */
                                        if (ant.steps_taken > 0
                                            && ant.previous_vertices
                                                       [ant.steps_taken - 1]
                                                   == candidate_vertex)
                                            continue;

                                        if (ant.did_backstep && ant.steps_taken > 0
                                            && ant.previous_vertices
                                                       [ant.steps_taken + 1]
                                                   == candidate_vertex)
                                            continue;

                                        total_score += calculate_score(
                                            map.vertex_coord_map[candidate_vertex], edge
                                        );
                                    }

                                    if (total_score == 0.0f) {
                                        return { false, {} };
                                    }

                                    f32 choice_val = (rand_val - exploitation_factor)
                                                     * total_score;
                                    for (auto edge : edges) {
                                        _VertexDescriptor candidate_vertex
                                            = boost::target(edge, map.graph);

                                        /**
                                         * If ant has just visited the candidate vertex,
                                         * then reject it as a candidate. "Just visited"
                                         * is the step earlier in all cases, but if the
                                         * ant has just stepped backwards this also
                                         * includes the next in previous_vertices from
                                         * the current step.
                                         */
                                        if (ant.steps_taken > 0
                                            && ant.previous_vertices
                                                       [ant.steps_taken - 1]
                                                   == candidate_vertex)
                                            continue;

                                        if (ant.did_backstep && ant.steps_taken > 0
                                            && ant.previous_vertices
                                                       [ant.steps_taken + 1]
                                                   == candidate_vertex)
                                            continue;

                                        choice_val -= map.pheromone_map[edge];

                                        if (choice_val <= 0.0f) {
                                            return { true, candidate_vertex };
                                        }
                                    }
                                }

                                debug_printf(
                                    "Error: could not decide where to send ant, check "
                                    "maths!"
                                );

                                return { false, {} };
                            };

                            auto [forward, next_vertex] = choose();
                            if (!forward) {
                                if (ant.steps_taken == 0) {
                                    debug_printf(
                                        "Could not step ant forward from initial "
                                        "vertex in "
                                        "BasicACS::find_path."
                                    );
                                    return;
                                }

                                ant.steps_taken  -= 1;
                                ant.did_backstep = true;
                                ant.current_vertex
                                    = ant.previous_vertices[ant.steps_taken];
                            } else {
                                // Do local pheromone update.
                                auto [edge, _] = boost::edge(
                                    ant.current_vertex, next_vertex, map.graph
                                );

                                map.pheromone_map[edge]
                                    = (1.0f - Config.local.evaporation)
                                          * map.pheromone_map[edge]
                                      + Config.local.evaporation
                                            * Config.local.increment;

                                // Update ant's vertex info.
                                ant.steps_taken                        += 1;
                                ant.did_backstep                       = false;
                                ant.previous_vertices[ant.steps_taken] = next_vertex;
                                ant.current_vertex                     = next_vertex;
                            }

                            bool need_new_group = false;
                            bool changed_group  = false;

                            // Increment cursor in ant's old group, use decremnted value
                            // in subsequent loop.
                            ant_group_cursors[ant.group] += 1;

                            for (size_t cursor = 0;
                                 cursor < ant_group_cursors[ant.group] - 1;
                                 ++cursor)
                            {
                                _Ant& companion_ant
                                    = *ant_groups_old.groups[ant.group].ants[cursor];

                                /**
                                 * If we find an ant from this ant's previous path group
                                 * who has moved to a new path group in this step, and
                                 * both ants have moved to the same vertex, then we
                                 * should put this current ant into the same path group.
                                 * We have to be careful about how we performed this
                                 * change, notes below detail the logic of how we ensure
                                 * things don't go weird. We break when we get here as
                                 * we have found the appropriate path group.
                                 */
                                if (companion_ant.current_vertex == ant.current_vertex
                                    && companion_ant.group != ant.group)
                                {
                                    // Place ant in same group as companion ant who has
                                    // taken same step.
                                    auto& new_group
                                        = ant_groups_new.groups[companion_ant.group];
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
                                 * If this current ant has moved to a different vertex
                                 * as an ant who was previously in the same path group
                                 * and who has already taken their step, then this
                                 * current ant no longer can belong to the same path
                                 * group and so should be marked to receive one. We
                                 * continue the loop however, in case another ant moved
                                 * to the same vertex as this current ant.
                                 */
                                if (companion_ant.current_vertex != ant.current_vertex
                                    && companion_ant.group == ant.group)
                                {
                                    need_new_group = true;
                                    changed_group  = false;
                                }
                            }

                            /**
                             * If this current ant is flagged as needing a new path
                             * group, then make it one and put it in it. While we don't
                             * have to worry about how it gets into the new path group,
                             * we still have to be careful of how it leaves its old one.
                             */
                            if (need_new_group) {
                                // Add new group on at end of list of groups, to avoid
                                // issues with groups that already exist.
                                auto& new_group
                                    = ant_groups_new.groups[ant_groups_new.count];
                                new_group.ants[new_group.size++] = &ant;

                                ant.group = ant_groups_new.count;

                                ant_groups_new.count += 1;
                            } else if (!changed_group) {
                                // Place ant in group is was in on last step, it hasn't
                                // moved.
                                auto& new_group = ant_groups_new.groups[ant.group];
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
                                        sizeof(_VertexDescriptor)
                                            * (ant.steps_taken + 1)
                                    );
                                }
                            }
                        }

                        // TODO(Matthew): remove by referring to groups by reference
                        // only.
                        std::memcpy(
                            &ant_groups_old, &ant_groups_new, sizeof(_AntGroups)
                        );

                        if constexpr (Config.debug.on) {
                            if (debugger) {
                                debugger->create_heatmaps(
                                    &ants[0], Config.ant_count, map
                                );
                            }
                        }

                        if (ants_found_food == Config.ant_count) break;
                    }

                    // Calculate entropy of iteration.
                    entropy = 0.0f;
                    for (size_t ant_group_idx = 0; ant_group_idx < ant_groups_new.count;
                         ++ant_group_idx)
                    {
                        f32 popularity
                            = static_cast<f32>(ant_groups_new.groups[ant_group_idx].size
                              )
                              / static_cast<f32>(Config.ant_count);
                        entropy += popularity * log(popularity);
                    }
                    entropy /= log(1.0f / static_cast<f32>(Config.ant_count));

                    // Global pheromone update.

                    // TODO(Matthew): can we do lazy evaluation of this?
                    for (auto edge :
                         boost::make_iterator_range(boost::edges(map.graph)))
                    {
                        map.pheromone_map[edge] *= (1.0f - Config.global.evaporation);
                    }

                    if (shortest_path.found) {
                        for (size_t step_idx = 0; step_idx < shortest_path.length - 1;
                             ++step_idx)
                        {
                            auto [edge, _] = boost::edge(
                                shortest_path.steps[step_idx],
                                shortest_path.steps[step_idx + 1],
                                map.graph
                            );

                            map.pheromone_map[edge]
                                += Config.global.evaporation
                                   * (Config.global.increment
                                      / static_cast<f32>(shortest_path.length));
                        }
                    }

                    // TODO(Matthew): does this work?
                    if (shortest_path.found
                        && glm::max(last_shortest_path, shortest_path.length)
                                   - glm::min(last_shortest_path, shortest_path.length)
                               <= Config.break_on_path_change)
                    {
                        satisfactory_change_its += 1;

                        if (satisfactory_change_its >= Config.break_on_iterations)
                            break;
                    } else {
                        satisfactory_change_its = 0;
                    }

                    last_shortest_path = shortest_path.length;
                }

                // TODO(Matthew): better...
                if (shortest_path.found) {
                    path_length = shortest_path.length;
                    path        = new Node[path_length];

                    for (size_t idx = 0; idx < path_length; ++idx) {
                        path[idx] = map.vertex_coord_map[shortest_path.steps[idx]];
                    }
                }
            }
        }  // namespace GraphACS
    }      // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_graph_acs_hpp

#ifndef __hemlock_algorithm_acs_graph_acs_hpp
#define __hemlock_algorithm_acs_graph_acs_hpp

#include "../state.hpp"
#include "view.hpp"

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
                GraphMapView<Node, IsWeighted>&      map_view,
                PheromoneMap<Node>&                  pheromone_map,
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

                using _GraphMap         = GraphMap<Node, IsWeighted>;
                using _EdgeDescriptor   = EdgeDescriptor<Node, IsWeighted>;
                using _VertexDescriptor = VertexDescriptor<Node, IsWeighted>;
                using _Ant              = Ant<_VertexDescriptor, Node, IsWeighted>;
                using _AntGroup
                    = AntGroup<_VertexDescriptor, Node, IsWeighted, Config.ant_count>;

                /******************\
                 * Initialisation *
                \******************/

                _GraphMap*        source_map = nullptr;
                _VertexDescriptor source_vertex, destination_vertex;

                std::tie(destination_vertex, source_map) = map_view.vertex(destination);
                std::tie(source_vertex, source_map)      = map_view.vertex(source);

                struct {
                    std::pair<_VertexDescriptor, _VertexDescriptor>
                               steps[Config.max_steps + 1] = {};
                    _GraphMap* maps[Config.max_steps + 1]  = {};
                    size_t     length = std::numeric_limits<size_t>::max();
                    bool       found  = false;
                } shortest_path;

                size_t last_shortest_path      = std::numeric_limits<size_t>::max();
                size_t satisfactory_change_its = 0;

                // TODO(Matthew): Heap allocation (paged?) at least for large number of
                // ants & max steps? Bucket of visited vertices for all ants. Vertices
                // visited by each ant so far on its path for an iteration.
                std::pair<_VertexDescriptor, _VertexDescriptor>
                    visited_vertices[Config.ant_count * (Config.max_steps + 1)]    = {};
                _GraphMap* visited_maps[Config.ant_count * (Config.max_steps + 1)] = {};

                // The actual ants.
                const _Ant nil_ants[Config.ant_count] = {};
                _Ant       ants[Config.ant_count]     = {};

                for (size_t ant_idx = 0; ant_idx < Config.ant_count; ++ant_idx) {
                    ants[ant_idx].current_vertex = { source_vertex, source_vertex };
                    ants[ant_idx].current_map    = source_map;
                }

                // TODO(Matthew): Do we want to handle large Config.ant_count? This
                // isn't okay for that. The ant groups used for calculating entropy.
                struct _AntGroups {
                    _AntGroup groups[Config.ant_count] = {};
                    size_t    count                    = 0;
                } ant_groups_new, ant_groups_old;

                // TODO(Matthew): local increment cannot be applied globally... are
                //                we sure this is even the correct implementation from
                //                papers?
                // for (auto edge : boost::make_iterator_range(boost::edges(map.graph)))
                // {
                //     pheromone_map[edge] += Config.local.increment;
                // }

                /********************\
                 * Pheromone Access *
                \********************/

                const auto pheromone_from_nodes
                    = [&](Node _source, Node _destination, size_t iteration) -> f32& {
                    f32 default_pheromone
                        = Config.local.increment
                          * glm::pow(
                              Config.global.evaporation, static_cast<f32>(iteration)
                          );

                    try {
                        auto source_pheromones = pheromone_map.at(_source);
                        try {
                            return source_pheromones.at(_destination);
                        } catch (std::out_of_range&) {
                            pheromone_map[_source][_destination] = default_pheromone;
                            return pheromone_map[_source][_destination];
                        }
                    } catch (std::out_of_range&) {
                        pheromone_map[_source]               = {};
                        pheromone_map[_source][_destination] = default_pheromone;
                        return pheromone_map[_source][_destination];
                    }
                };

                const auto pheromone_from_vertices = [&](_VertexDescriptor _source,
                                                         _VertexDescriptor _destination,
                                                         _GraphMap&        map,
                                                         size_t iteration) -> f32& {
                    return pheromone_from_nodes(
                        map.vertex_coord_map[_source],
                        map.vertex_coord_map[_destination],
                        iteration
                    );
                };

                const auto pheromone_from_edge
                    = [&](_EdgeDescriptor edge, _GraphMap& map, size_t iteration
                      ) -> f32& {
                    return pheromone_from_nodes(
                        map.vertex_coord_map[boost::source(edge, map.graph)],
                        map.vertex_coord_map[boost::target(edge, map.graph)],
                        iteration
                    );
                };

                /*********************\
                 * Score Calculation *
                \*********************/

                const auto calculate_score = [&](Node            candidate,
                                                 _EdgeDescriptor edge,
                                                 _GraphMap&      map,
                                                 size_t          iteration) {
                    // TODO(Matthew): Fully expose this (at least optionally), likewise
                    // make even providing distance calc
                    //                and modifying score with it optional.

                    const DistanceCalculator calc_distance{};

                    f32 score = pheromone_from_edge(edge, map, iteration);

                    score
                        /= (1.0f + calc_distance(map, source, destination, candidate));

                    return score;
                };

                /*****************\
                 * Do Iterations *
                \*****************/

                // Start off assuming minimal entropy, causes more exploration.
                f32 entropy = 0.0f;

                for (size_t iteration = 0; iteration < Config.max_iterations;
                     ++iteration)
                {
                    for (size_t i = 0; i < Config.ant_count; ++i) {
                        ants[i] = nil_ants[i];
                    }
                    ant_groups_old = {};
                    for (size_t ant_idx = 0; ant_idx < Config.ant_count; ++ant_idx) {
                        ants[ant_idx].current_vertex = { source_vertex, source_vertex };
                        ants[ant_idx].current_map    = source_map;
                        ants[ant_idx].previous_vertices
                            = &visited_vertices[ant_idx * (Config.max_steps + 1)];
                        ants[ant_idx].previous_vertices[0]
                            = { source_vertex, source_vertex };
                        ants[ant_idx].previous_maps
                            = &visited_maps[ant_idx * (Config.max_steps + 1)];
                        ants[ant_idx].previous_maps[0]         = source_map;
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
                                auto edges
                                    = boost::make_iterator_range(boost::out_edges(
                                        ant.current_vertex.second,
                                        ant.current_map->graph
                                    ));

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
                                            = boost::target(
                                                edge, ant.current_map->graph
                                            );

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
                                                           .second
                                                   == candidate_vertex)
                                            continue;

                                        if (ant.did_backstep && ant.steps_taken > 0
                                            && ant.previous_vertices
                                                       [ant.steps_taken + 1]
                                                           .second
                                                   == candidate_vertex)
                                            continue;

                                        f32 score = calculate_score(
                                            ant.current_map
                                                ->vertex_coord_map[candidate_vertex],
                                            edge,
                                            *ant.current_map,
                                            iteration
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
                                            = boost::target(
                                                edge, ant.current_map->graph
                                            );

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
                                                           .second
                                                   == candidate_vertex)
                                            continue;

                                        if (ant.did_backstep && ant.steps_taken > 0
                                            && ant.previous_vertices
                                                       [ant.steps_taken + 1]
                                                           .second
                                                   == candidate_vertex)
                                            continue;

                                        total_score += calculate_score(
                                            ant.current_map
                                                ->vertex_coord_map[candidate_vertex],
                                            edge,
                                            *ant.current_map,
                                            iteration
                                        );
                                    }

                                    if (total_score == 0.0f) {
                                        return { false, {} };
                                    }

                                    f32 choice_val = (rand_val - exploitation_factor)
                                                     * total_score;
                                    for (auto edge : edges) {
                                        _VertexDescriptor candidate_vertex
                                            = boost::target(
                                                edge, ant.current_map->graph
                                            );

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
                                                           .second
                                                   == candidate_vertex)
                                            continue;

                                        if (ant.did_backstep && ant.steps_taken > 0
                                            && ant.previous_vertices
                                                       [ant.steps_taken + 1]
                                                           .second
                                                   == candidate_vertex)
                                            continue;

                                        choice_val -= pheromone_from_edge(
                                            edge, *ant.current_map, iteration
                                        );

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
                                ant.current_map = ant.previous_maps[ant.steps_taken];
                            } else {
                                // Do local pheromone update.
                                auto [edge, _] = boost::edge(
                                    ant.current_vertex.second,
                                    next_vertex,
                                    ant.current_map->graph
                                );

                                pheromone_from_edge(edge, *ant.current_map, iteration)
                                    = (1.0f - Config.local.evaporation)
                                          * pheromone_from_edge(
                                              edge, *ant.current_map, iteration
                                          )
                                      + Config.local.evaporation
                                            * Config.local.increment;

                                // Update ant's vertex info.
                                ant.steps_taken  += 1;
                                ant.did_backstep = false;

                                // Need to check if the next vertex is possibly in a new
                                // graph, otherwise we'll have mismatching vertex and
                                // graph references.
                                auto [new_vertex, new_map] = map_view.vertex(
                                    ant.current_map->vertex_coord_map[next_vertex]
                                );
                                ant.current_vertex = { next_vertex, new_vertex };
                                ant.previous_vertices[ant.steps_taken]
                                    = { next_vertex, new_vertex };
                                ant.current_map                    = new_map;
                                ant.previous_maps[ant.steps_taken] = new_map;
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

                            if (ant.current_map
                                    ->vertex_coord_map[ant.current_vertex.second]
                                == destination)
                            {
                                ant.found_food  = true;
                                ants_found_food += 1;

                                if (ant.steps_taken + 1 < shortest_path.length) {
                                    shortest_path.length = ant.steps_taken + 1;
                                    shortest_path.found  = true;
                                    for (size_t i = 0; i < ant.steps_taken + 1; ++i) {
                                        shortest_path.steps[i]
                                            = ant.previous_vertices[i];
                                        shortest_path.maps[i] = ant.previous_maps[i];
                                    }
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
                                    &ants[0], Config.ant_count, source_map
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
                    debug_printf(
                        "Not doing global evaporation due to needing to find a way to "
                        "do it lazily on at least a per-map level."
                    );
                    // for (auto edge :
                    //      boost::make_iterator_range(boost::edges(map.graph)))
                    // {
                    //     map.pheromone_map[edge] *= (1.0f -
                    //     Config.global.evaporation);
                    // }

                    if (shortest_path.found) {
                        for (size_t step_idx = 0; step_idx < shortest_path.length - 1;
                             ++step_idx)
                        {
                            pheromone_from_vertices(
                                shortest_path.steps[step_idx].second,
                                shortest_path.steps[step_idx + 1].first,
                                *shortest_path.maps[step_idx],
                                iteration
                            ) += Config.global.evaporation
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
                        path[idx]
                            = shortest_path.maps[idx]
                                  ->vertex_coord_map[shortest_path.steps[idx].second];
                    }
                }
            }
        }  // namespace GraphACS
    }      // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_algorithm_acs_graph_acs_hpp

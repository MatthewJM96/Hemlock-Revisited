#ifndef __hemlock_tests_test_navmesh_screen_hpp
#define __hemlock_tests_test_navmesh_screen_hpp

#include <FastNoise/FastNoise.h>

#include "algorithm/acs/acs.hpp"
#include "memory/handle.hpp"
#include "voxel/ai/navmesh.hpp"
#include "voxel/generation/generator_task.hpp"
#include "voxel/graphics/mesh/greedy_strategy.hpp"
#include "voxel/graphics/mesh/mesh_task.hpp"
#include "voxel/graphics/outline_renderer.hpp"
#include "voxel/ray.h"

#include "tests/iomanager.hpp"

#if defined(DEBUG)
#  define VIEW_DIST 4
#else
#  define VIEW_DIST 10
#endif

#include "tests/navmesh_screen/io.hpp"
#include "tests/navmesh_screen/terrain.hpp"

struct TNS_ChunkOutlinePredicate {
    std::tuple<bool, colour4> operator()(hmem::Handle<hvox::Chunk>) {
        return {
            true, {255, 255, 255, 255}
        };
    }
};

struct TNS_ACSDistanceCalculator {
    f32 operator()(
        const halgo::GraphMap<hvox::ai::ChunkNavmeshNode, false>&,
        const hvox::ai::ChunkNavmeshNode&,
        const hvox::ai::ChunkNavmeshNode& destination,
        const hvox::ai::ChunkNavmeshNode& candidate
    ) const {
        hvox::BlockWorldPosition destination_world
            = hvox::block_world_position(destination.chunk_pos, destination.block_pos);
        hvox::BlockWorldPosition candidate_world
            = hvox::block_world_position(candidate.chunk_pos, candidate.block_pos);

        f32v3 candidate_vect = static_cast<f32v3>(destination_world)
                               - static_cast<f32v3>(candidate_world);

        return glm::dot(candidate_vect, candidate_vect);
    }
};

class TestNavmeshScreen : public happ::ScreenBase {
public:
    TestNavmeshScreen() : happ::ScreenBase(), m_input_manager(nullptr) { /* Empty. */
    }

    virtual ~TestNavmeshScreen(){
        /* Empty. */
    };

    virtual void dispose() override {
        m_chunk_outline_renderer.dispose();

        m_block_outline_renderer.dispose();

        m_navmesh_outline_renderer.dispose();

        m_chunk_grid->dispose();

        happ::ScreenBase::dispose();
    }

    virtual void start(hemlock::FrameTime time) override {
        happ::ScreenBase::start(time);

        htest::navmesh_screen::load_chunks(m_chunk_grid, &m_navmesh_outline_renderer);

        hui::InputDispatcher::instance()->on_mouse.button_down += &handle_mouse_button;
        hui::InputDispatcher::instance()->on_mouse.move        += &handle_mouse_move;
        // hui::InputDispatcher::instance()->on_keyboard.button_down +=
        // &handle_key_down;
    }

    virtual void update(hemlock::FrameTime time) override {
        static f32v3 last_pos{ 0.0f };

        f32 frame_time = hemlock::frame_time_to_floating<>(time);

        f32   speed_mult = 1.0f;
        f32v3 delta_pos  = {};
        htest::navmesh_screen::handle_simple_user_inputs(
            m_input_manager, m_camera, frame_time, speed_mult, delta_pos
        );

#if defined(DEBUG)
        static f64 countdown = 1000.0;
        countdown            -= static_cast<f64>(
            std::chrono::duration_cast<std::chrono::milliseconds>(time).count()
        );
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_T)) {
            if (countdown < 0.0) {
                f32v3 pos = m_camera.position();
                f32v3 dir = m_camera.direction();
                debug_printf(
                    "Camera Coords: (%f, %f, %f)\nCamera Direction: (%f, %f, %f)\n",
                    pos.x,
                    pos.y,
                    pos.z,
                    dir.x,
                    dir.y,
                    dir.z
                );
            }
        }
        if (countdown < 0.0) countdown = 1000.0;
#endif

        m_camera.offset_position(f32v3{ delta_pos.x, delta_pos.y, delta_pos.z });
        m_camera.update();

        f32v3 current_pos
            = glm::floor(m_camera.position() / static_cast<f32>(CHUNK_LENGTH));

        htest::navmesh_screen::unload_x_chunks(
            m_chunk_grid,
            m_unloading_chunks,
            current_pos,
            last_pos,
            &m_navmesh_outline_renderer
        );

        htest::navmesh_screen::unload_z_chunks(
            m_chunk_grid,
            m_unloading_chunks,
            current_pos,
            last_pos,
            &m_navmesh_outline_renderer
        );

        last_pos = current_pos;

        m_chunk_grid->update(time);

        if (m_nav_test_mode == 3) {
            auto chunk_start = m_nav_test_start.chunk.lock();
            auto chunk_end   = m_nav_test_end.chunk.lock();
            if (chunk_start != nullptr && chunk_end != nullptr) {
                hmem::SharedResourceLock lock_start, lock_end;
                auto                     navmesh_start
                    = chunk_start->navmesh.get(lock_start, std::defer_lock);
                auto& navmesh_end = chunk_end->navmesh.get(lock_end, std::defer_lock);
                std::lock(lock_start, lock_end);

                // constexpr halgo::ACSConfig Config = { .debug = { .on = false } };

                hvox::ai::ChunkNavmeshNode start
                    = { hvox::block_chunk_position(m_nav_test_start.pos),
                        chunk_start->position };
                hvox::ai::ChunkNavmeshNode end
                    = { hvox::block_chunk_position(m_nav_test_end.pos),
                        chunk_end->position };

                if (navmesh_start.data->coord_vertex_map.find(start)
                    != navmesh_start.data->coord_vertex_map.end())
                {
                    auto edges = boost::make_iterator_range(boost::out_edges(
                        navmesh_start.data->coord_vertex_map[start],
                        navmesh_start.data->graph
                    ));

                    std::cout << "From green:" << std::endl;
                    size_t i = 1;
                    for (auto edge : edges) {
                        hvox::ai::ChunkNavmeshNode target
                            = navmesh_start.data->vertex_coord_map[boost::target(
                                edge, navmesh_start.data->graph
                            )];

                        std::cout << "  " << std::to_string(i++) << " - {"
                                  << std::to_string(target.block_pos.x) << ", "
                                  << std::to_string(target.block_pos.y) << ", "
                                  << std::to_string(target.block_pos.z) << "}"
                                  << " - {" << std::to_string(target.chunk_pos.x)
                                  << ", " << std::to_string(target.chunk_pos.y) << ", "
                                  << std::to_string(target.chunk_pos.z) << "}"
                                  << std::endl;

                        m_block_outline_renderer.add_outline(hvox::OutlineData{
                            static_cast<f32v3>(hvox::block_world_position(
                                target.chunk_pos, target.block_pos
                            )),
                            {0, 0, 255, 255}
                        });
                    }
                }

                if (navmesh_end.data->coord_vertex_map.find(end)
                    != navmesh_end.data->coord_vertex_map.end())
                {
                    auto edges = boost::make_iterator_range(boost::out_edges(
                        navmesh_end.data->coord_vertex_map[end], navmesh_end.data->graph
                    ));

                    std::cout << "From red:" << std::endl;
                    size_t i = 1;
                    for (auto edge : edges) {
                        hvox::ai::ChunkNavmeshNode target
                            = navmesh_end.data->vertex_coord_map[boost::target(
                                edge, navmesh_end.data->graph
                            )];

                        std::cout << "  " << std::to_string(i++) << " - {"
                                  << std::to_string(target.block_pos.x) << ", "
                                  << std::to_string(target.block_pos.y) << ", "
                                  << std::to_string(target.block_pos.z) << "}"
                                  << " - {" << std::to_string(target.chunk_pos.x)
                                  << ", " << std::to_string(target.chunk_pos.y) << ", "
                                  << std::to_string(target.chunk_pos.z) << "}"
                                  << std::endl;

                        m_block_outline_renderer.add_outline(hvox::OutlineData{
                            static_cast<f32v3>(hvox::block_world_position(
                                target.chunk_pos, target.block_pos
                            )),
                            {0, 0, 255, 255}
                        });
                    }
                }

                // hvox::ai::ChunkNavmeshNode* path        = nullptr;
                // size_t                      path_length = 0;

                // halgo::GraphACS::find_path<
                //     hvox::ai::ChunkNavmeshNode,
                //     false,
                //     Config,
                //     TNS_ACSDistanceCalculator>(
                //     chunk->navmesh, start, end, path, path_length
                // );

                // if (path) {
                //     std::cout << "Path length is: " << std::to_string(path_length)
                //               << std::endl;

                //     for (size_t i = 0; i < path_length; ++i) {
                //         std::cout << "  " << std::to_string(i) << " - {"
                //                   << std::to_string(path[i].block_pos.x) << ", "
                //                   << std::to_string(path[i].block_pos.y) << ", "
                //                   << std::to_string(path[i].block_pos.z) << "}"
                //                   << std::endl;
                //         m_block_outline_renderer.add_outline(hvox::OutlineData{
                //             static_cast<f32v3>(hvox::block_world_position(
                //                 path[i].chunk_pos, path[i].block_pos
                //             )),
                //             {0, 0, 255, 255}
                //         });
                //     }
                // } else {
                //     std::cout << "Path not found!" << std::endl;
                // }
            }

            m_nav_test_mode = 0;
        }

        m_navmesh_outline_renderer.update(time);
    }

    virtual void draw(hemlock::FrameTime time) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_shader.use();

        glUniformMatrix4fv(
            m_shader.uniform_location("view_proj"),
            1,
            GL_FALSE,
            &m_camera.view_projection_matrix()[0][0]
        );

        glBindTextureUnit(0, m_default_texture);
        glUniform1i(m_shader.uniform_location("tex"), 0);

        m_chunk_grid->draw(time);

        // Deactivate our shader.
        m_shader.unuse();

        m_line_shader.use();

        f32v3 line_colour{ 1.0f };

        auto dims = m_process->window()->dimensions();

        // Note would better be done only on window dimension change, but lazy.
        {
            std::vector<f32v3> lines;
            lines.emplace_back(f32v3{
                dims.width / 2.0f - 20.0f, dims.height / 2.0f, 0.0f });
            lines.emplace_back(f32v3{
                dims.width / 2.0f + 20.0f, dims.height / 2.0f, 0.0f });
            lines.emplace_back(f32v3{
                dims.width / 2.0f, dims.height / 2.0f - 20.0f, 0.0f });
            lines.emplace_back(f32v3{
                dims.width / 2.0f, dims.height / 2.0f + 20.0f, 0.0f });

            glNamedBufferSubData(
                m_crosshair_vbo,
                0,
                lines.size() * sizeof(f32v3),
                reinterpret_cast<void*>(&lines[0])
            );
        }

        f32m4 mvp = glm::ortho(
            0.0f, static_cast<f32>(dims.width), 0.0f, static_cast<f32>(dims.height)
        );

        glUniformMatrix4fv(
            m_line_shader.uniform_location("view_proj"), 1, GL_FALSE, &mvp[0][0]
        );
        glUniform3fv(m_line_shader.uniform_location("colour"), 1, &line_colour[0]);

        glBindVertexArray(m_crosshair_vao);
        glDrawArrays(GL_LINES, 0, 4);

        m_line_shader.unuse();

        if (m_draw_chunk_outlines || m_draw_block_outlines) {
            m_outline_shader.use();

            glUniformMatrix4fv(
                m_outline_shader.uniform_location("view_proj"),
                1,
                GL_FALSE,
                &m_camera.view_projection_matrix()[0][0]
            );

            if (m_draw_chunk_outlines) m_chunk_outline_renderer.draw(time);

            if (m_draw_block_outlines) m_block_outline_renderer.draw(time);

            m_outline_shader.unuse();
        }

        if (m_draw_navmesh_outlines) {
            m_navmesh_outline_shader.use();

            glUniformMatrix4fv(
                m_navmesh_outline_shader.uniform_location("view_proj"),
                1,
                GL_FALSE,
                &m_camera.view_projection_matrix()[0][0]
            );

            m_navmesh_outline_renderer.draw(time);

            m_navmesh_outline_shader.unuse();
        }
    }

    virtual void init(const std::string& name, happ::ProcessBase* process) override {
        happ::ScreenBase::init(name, process);

        m_state = happ::ScreenState::RUNNING;

        m_input_manager
            = static_cast<happ::SingleWindowApp*>(m_process)->input_manager();
        m_input_manager->init();

        m_capture_mouse = false;

        m_camera.attach_to_window(m_process->window());
        m_camera.set_position(f32v3{ 0.0f, 60.0f, 0.0f });
        m_camera.rotate_from_mouse_with_absolute_up(-130.0f, 160.0f, 0.005f);
        m_camera.set_fov(90.0f);
        m_camera.update();

        m_shader_cache.init(
            &m_iom,
            hg::ShaderCache::Parser{
                [](const hio::fs::path& path, hio::IOManagerBase* iom) -> std::string {
                    std::string buffer;
                    if (!iom->read_file_to_string(path, buffer)) return "";

                    return buffer;
                } }
        );

        m_shader.init(&m_shader_cache);
        // m_shader.set_attribute("v_position",      0);
        // m_shader.set_attribute("v_texture_coord", 1);
        // m_shader.set_attribute("v_normal",        2);
        m_shader.add_shaders("shaders/test_vox.vert", "shaders/test_vox.frag");
        m_shader.link();

        m_line_shader.init(&m_shader_cache);
        // m_line_shader.set_attribute("v_position",      0);
        m_line_shader.add_shaders("shaders/line.vert", "shaders/line.frag");
        m_line_shader.link();

        m_draw_chunk_outlines = false;
        m_draw_block_outlines = true;
        m_outline_shader.init(&m_shader_cache);
        m_outline_shader.add_shaders(
            "shaders/chunk_outline.vert", "shaders/chunk_outline.frag"
        );
        m_outline_shader.link();

        m_draw_navmesh_outlines = true;
        m_navmesh_outline_shader.init(&m_shader_cache);
        m_navmesh_outline_shader.add_shaders(
            "shaders/navmesh_outline.vert", "shaders/navmesh_outline.frag"
        );
        m_navmesh_outline_shader.link();

        m_default_texture = hg::load_texture("test_tex.png");

        static auto navmesh_task_builder = hvox::ChunkTaskBuilder{ []() {
            return new hvox::ai::ChunkNavmeshTask<
                hvox::ai::NaiveNavmeshStrategy<htest::navmesh_screen::BlockSolidCheck>>(
            );
        } };

        m_chunk_grid = hmem::make_handle<hvox::ChunkGrid>();
        m_chunk_grid->init(
            m_chunk_grid,
            VIEW_DIST * 2 + 1,
            28,
            hvox::ChunkTaskBuilder{ []() {
                return new hvox::ChunkGenerationTask<
                    htest::navmesh_screen::VoxelGenerator>();
            } },
            hvox::ChunkTaskBuilder{ []() {
                return new hvox::ChunkMeshTask<
                    hvox::GreedyMeshStrategy<htest::navmesh_screen::BlockComparator>>();
            } },
            &navmesh_task_builder
        );

        m_nav_test_mode  = 0;
        m_nav_test_start = { 0, {}, {} };
        m_nav_test_end   = { 0, {}, {} };

        m_chunk_outline_renderer.init(TNS_ChunkOutlinePredicate{}, m_chunk_grid);

        m_block_outline_renderer.init();

        m_navmesh_outline_renderer.init();

        glCreateVertexArrays(1, &m_crosshair_vao);

        glCreateBuffers(1, &m_crosshair_vbo);
        glNamedBufferData(
            m_crosshair_vbo,
            sizeof(f32v3) * 2 * 2,  // 2 points per line, 2 lines for crosshair.
            nullptr,
            GL_DYNAMIC_DRAW
        );

        glVertexArrayVertexBuffer(
            m_crosshair_vao, 0, m_crosshair_vbo, 0, sizeof(f32v3)
        );

        glEnableVertexArrayAttrib(m_crosshair_vao, 0);
        glVertexArrayAttribFormat(m_crosshair_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_crosshair_vao, 0, 0);

        handle_mouse_button = hemlock::Subscriber<
            hui::MouseButtonEvent>{ [&](hemlock::Sender, hui::MouseButtonEvent ev) {
            if (ev.button_id == static_cast<ui8>(hui::MouseButton::LEFT)) {
                if (m_nav_test_mode > 0 && m_nav_test_mode < 3) {
                    hvox::BlockWorldPosition      position;
                    f32                           distance;
                    hmem::WeakHandle<hvox::Chunk> chunk;

                    if (hvox::Ray::cast_to_block(
                            m_camera.position(),
                            m_camera.direction(),
                            m_chunk_grid,
                            hvox::Block{ 1 },
                            10,
                            position,
                            distance,
                            &chunk
                        ))
                    {
                        if (m_nav_test_mode == 1) {
                            if (m_nav_test_start.outline_id > 0) {
                                m_block_outline_renderer.modify_outline(
                                    m_nav_test_start.outline_id,
                                    hvox::OutlineData{
                                        static_cast<f32v3>(position),
                                        {0, 255, 0, 255}
                                }
                                );

                                m_nav_test_start.pos   = position;
                                m_nav_test_start.chunk = chunk;
                            } else {
                                size_t id = m_block_outline_renderer.add_outline(
                                    hvox::OutlineData{
                                        static_cast<f32v3>(position),
                                        {0, 255, 0, 255}
                                }
                                );

                                m_nav_test_start = { id, position, chunk };
                            }
                        } else if (m_nav_test_mode == 2) {
                            if (m_nav_test_end.outline_id > 0) {
                                m_block_outline_renderer.modify_outline(
                                    m_nav_test_end.outline_id,
                                    hvox::OutlineData{
                                        static_cast<f32v3>(position),
                                        {255, 0, 0, 255}
                                }
                                );

                                m_nav_test_end.pos   = position;
                                m_nav_test_end.chunk = chunk;
                            } else {
                                size_t id = m_block_outline_renderer.add_outline(
                                    hvox::OutlineData{
                                        static_cast<f32v3>(position),
                                        {255, 0, 0, 255}
                                }
                                );

                                m_nav_test_end = { id, position, chunk };
                            }
                        }

                        ++m_nav_test_mode;
                    }
                } /* else {
                     hvox::BlockWorldPosition position;
                     f32                      distance;

                     if (hvox::Ray::cast_to_block_before(
                             m_camera.position(),
                             m_camera.direction(),
                             m_chunk_grid,
                             hvox::Block{ 1 },
                             10,
                             position,
                             distance
                         ))
                     {
                         auto chunk
                             = m_chunk_grid->chunk(hvox::chunk_grid_position(position
                             ));

                         if (chunk != nullptr) {
                             hvox::set_block(
                                 chunk,
                                 hvox::block_chunk_position(position),
                                 hvox::Block{ 1 }
                             );
                         }
                     }
                 }*/
            }
        } };

        handle_key_down = hemlock::Subscriber<hui::KeyboardButtonEvent>{
            [&](hemlock::Sender, hui::KeyboardButtonEvent ev) {
                if (ev.physical_key == hui::PhysicalKey::H_RETURN) {
                    m_capture_mouse = true;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                } else if (ev.physical_key == hui::PhysicalKey::H_ESCAPE) {
                    m_capture_mouse = false;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                } else if (ev.physical_key == hui::PhysicalKey::H_L) {
                    m_draw_chunk_outlines = !m_draw_chunk_outlines;
                } else if (ev.physical_key == hui::PhysicalKey::H_T) {
                    if (m_nav_test_mode == 0) m_nav_test_mode = 1;
                } else if (ev.physical_key == hui::PhysicalKey::H_K) {
                    m_draw_navmesh_outlines = !m_draw_navmesh_outlines;
                }
            }
        };
        hui::InputDispatcher::instance()->on_keyboard.button_down += &handle_key_down;

        handle_mouse_move
            = hemlock::Subscriber<hui::MouseMoveEvent>{ [&](hemlock::Sender,
                                                            hui::MouseMoveEvent ev) {
                  if (m_capture_mouse) {
                      m_camera.rotate_from_mouse_with_absolute_up(
                          -1.0f * static_cast<f32>(ev.dx),
                          -1.0f * static_cast<f32>(ev.dy),
                          0.005f
                      );
                  }
              } };
    }
protected:
    hemlock::Subscriber<hui::MouseMoveEvent>      handle_mouse_move;
    hemlock::Subscriber<hui::MouseButtonEvent>    handle_mouse_button;
    hemlock::Subscriber<hui::KeyboardButtonEvent> handle_key_down;

    ui32 m_default_texture;

    MyIOManager                   m_iom;
    hg::ShaderCache               m_shader_cache;
    hcam::BasicFirstPersonCamera  m_camera;
    hui::InputManager*            m_input_manager;
    hmem::Handle<hvox::ChunkGrid> m_chunk_grid;
    hg::GLSLProgram m_shader, m_line_shader, m_outline_shader, m_navmesh_outline_shader;

    bool m_capture_mouse;

    hvox::ConditionalChunkOutlineRenderer<TNS_ChunkOutlinePredicate>
         m_chunk_outline_renderer;
    bool m_draw_chunk_outlines;

    hvox::BlockOutlineRenderer m_block_outline_renderer;
    bool                       m_draw_block_outlines;

    hvox::NavmeshOutlineRenderer m_navmesh_outline_renderer;
    bool                         m_draw_navmesh_outlines;

    ui32 m_nav_test_mode;

    struct {
        size_t                        outline_id;
        hvox::BlockWorldPosition      pos;
        hmem::WeakHandle<hvox::Chunk> chunk;
    } m_nav_test_start, m_nav_test_end;

    GLuint m_crosshair_vao, m_crosshair_vbo;

    std::vector<hmem::WeakHandle<hvox::Chunk>> m_unloading_chunks;
};

#undef VIEW_DIST

#endif  // __hemlock_tests_test_navmesh_screen_hpp

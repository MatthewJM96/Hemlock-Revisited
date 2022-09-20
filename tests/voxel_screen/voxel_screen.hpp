#ifndef __hemlock_tests_test_voxel_screen_hpp
#define __hemlock_tests_test_voxel_screen_hpp

#include <bullet/BulletCollision/CollisionShapes/btBoxShape.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <FastNoise/FastNoise.h>

#include "memory/handle.hpp"
#include "voxel/chunk/generator_task.hpp"
#include "voxel/chunk/mesh/greedy_task.hpp"
#include "voxel/ray.h"

#include "physics/voxel/chunk_grid_collider.hpp"

#include "tests/iomanager.hpp"


#include "algorithm/acs/acs.hpp"


#if defined(DEBUG)
#   define VIEW_DIST 4
#else
#   define VIEW_DIST 10
#endif

#include "tests/voxel_screen/io.hpp"
#include "tests/voxel_screen/physics.hpp"
#include "tests/voxel_screen/player.hpp"
#include "tests/voxel_screen/terrain.hpp"



class TestVoxelScreen : public happ::ScreenBase {
public:
    TestVoxelScreen() :
        happ::ScreenBase(),
        m_input_manager(nullptr)
    { /* Empty. */ }
    virtual ~TestVoxelScreen() { /* Empty */ };

    virtual void start(hemlock::FrameTime time) override {
        happ::ScreenBase::start(time);

        htest::voxel_screen::load_chunks(m_chunk_grid);
    }

    virtual void update(hemlock::FrameTime time) override {
        static f32v3 last_pos{0.0f};


        static bool do_chunk_check  = false;
        static bool do_unloads      = false;

        f32 frame_time = hemlock::frame_time_to_floating<>(time);

        bool  flip_chunk_check  = false;
        f32   speed_mult        = 1.0f;
        f32v3 delta_pos         = {};
        htest::voxel_screen::handle_simple_user_inputs(
            m_input_manager,
            m_camera,
            m_phys.world,
            frame_time,
            flip_chunk_check,
            m_draw_chunk_outlines,
            do_unloads,
            speed_mult,
            delta_pos
        );

        if (do_chunk_check && !flip_chunk_check) return;
        if (flip_chunk_check) do_chunk_check = !do_chunk_check;

        if (do_unloads) htest::voxel_screen::unload_chunks(m_chunk_grid, frame_time);

#if defined(DEBUG)
        static f64 countdown = 1000.0;
        countdown -= static_cast<f64>(std::chrono::duration_cast<std::chrono::milliseconds>(time).count());
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_T)) {
            if (countdown < 0.0) {
                f32v3 pos = m_camera.position();
                f32v3 dir = m_camera.direction();
                debug_printf("Camera Coords: (%f, %f, %f)\nCamera Direction: (%f, %f, %f)\n", pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);
            }
        }
        if (countdown < 0.0) countdown = 1000.0;
#endif

        m_player.ac.position += hvox::EntityWorldPosition{f32v3{delta_pos.x, 0.0f, delta_pos.z} * static_cast<f32>(1ll << 32)};
        m_camera.offset_position(f32v3{delta_pos.x, 0.0f, delta_pos.z});
        {
            auto transform = m_player.body->getWorldTransform();
            transform.setOrigin(btVector3(m_camera.position().x, m_camera.position().y, m_camera.position().z));
            m_player.body->setWorldTransform(transform);
        }
        m_camera.update();

        f32v3 current_pos = glm::floor(m_camera.position() / static_cast<f32>(CHUNK_LENGTH));

        htest::voxel_screen::unload_x_chunks(
            m_chunk_grid,
            m_unloading_chunks,
            current_pos,
            last_pos
        );

        htest::voxel_screen::unload_z_chunks(
            m_chunk_grid,
            m_unloading_chunks,
            current_pos,
            last_pos
        );

        last_pos = current_pos;

        m_chunk_grid->update(time);

        static btRigidBody*     voxel_patch_body    = nullptr;
        static btCompoundShape* voxel_patch         = nullptr;

        if (voxel_patch_body) {
            m_phys.world->removeRigidBody(voxel_patch_body);
            delete voxel_patch_body;
            voxel_patch_body = nullptr;
        }
        if (voxel_patch) {
            delete voxel_patch;
            voxel_patch = nullptr;
        }

        voxel_patch = new btCompoundShape();
        if (
            hphys::ChunkGridCollider::determine_candidate_colliding_voxels<
                htest::voxel_screen::TVS_VoxelShapeEvaluator
            >(m_player.ac, m_player.dc, m_player.cc, voxel_patch)
        ) {
            btTransform transform = btTransform::getIdentity();
            transform.setOrigin(btVector3(glm::floor(m_camera.position().x), glm::floor(m_camera.position().y), glm::floor(m_camera.position().z)));
            btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
            btVector3 inertia;
            btScalar mass = 0.0f;
            voxel_patch->calculateLocalInertia(mass, inertia);
            btRigidBody::btRigidBodyConstructionInfo body_info = btRigidBody::btRigidBodyConstructionInfo(mass, motion_state, voxel_patch, inertia);
            body_info.m_restitution = 0.0f;
            body_info.m_friction = 1.0f;
            voxel_patch_body = new btRigidBody(body_info);
            m_phys.world->addRigidBody(voxel_patch_body);
        }

        m_player.body->activate();
        m_phys.world->stepSimulation(hemlock::frame_time_to_floating<std::chrono::seconds, btScalar>(time));

        m_camera.set_position(f32v3{
            m_player.body->getWorldTransform().getOrigin().x(),
            m_player.body->getWorldTransform().getOrigin().y(),
            m_player.body->getWorldTransform().getOrigin().z()
        });
        m_player.ac.position = hvox::EntityWorldPosition{
            static_cast<hvox::EntityWorldPositionCoord>(m_camera.position().x * static_cast<f32>(1ll << 32)),
            static_cast<hvox::EntityWorldPositionCoord>(m_camera.position().y * static_cast<f32>(1ll << 32)),
            static_cast<hvox::EntityWorldPositionCoord>(m_camera.position().z * static_cast<f32>(1ll << 32))
        };

        if (do_chunk_check) {
            debug_printf("Frame time: %f\n", hemlock::frame_time_to_floating<>(time));

            debug_printf("Camera at:         (%f, %f, %f)\n", m_camera.position().x, m_camera.position().y, m_camera.position().z);
            debug_printf("Camera looking at: (%f, %f, %f)\n", m_camera.direction().x, m_camera.direction().y, m_camera.direction().z);

            auto _voxel_patch = new btCompoundShape();
            if (
                hphys::ChunkGridCollider::determine_candidate_colliding_voxels<
                    htest::voxel_screen::TVS_VoxelShapeEvaluator
                >(m_player.ac, m_player.dc, m_player.cc, _voxel_patch)
            ) {
                btVector3 min_aabb, max_aabb;
                _voxel_patch->getAabb(btTransform::getIdentity(), min_aabb, max_aabb);

                debug_printf("Voxel patch spans:\n    (%f, %f, %f)\n    (%f, %f, %f)\n",
                            min_aabb.x() + m_camera.position().x,
                            min_aabb.y() + m_camera.position().y,
                            min_aabb.z() + m_camera.position().z,
                            max_aabb.x() + m_camera.position().x,
                            max_aabb.y() + m_camera.position().y,
                            max_aabb.z() + m_camera.position().z
                );
            } else {
                debug_printf("No voxels in voxel patch.\n");
            }

            debug_printf("Chunks still not unloaded:\n");
            for (auto& handle : m_unloading_chunks) {
                if (handle.expired()) continue;

                auto chunk = handle.lock();

                if (
                    (chunk->position.x < static_cast<i32>(current_pos.x) - VIEW_DIST) || (chunk->position.x > static_cast<i32>(current_pos.x) - VIEW_DIST) ||
                    (chunk->position.z < static_cast<i32>(current_pos.z) - VIEW_DIST) || (chunk->position.z > static_cast<i32>(current_pos.z) - VIEW_DIST)
                ) {
                    debug_printf("    (%d, %d, %d)\n", chunk->position.x, chunk->position.y, chunk->position.z);
                }
            }
        }
    }
    virtual void draw(hemlock::FrameTime time) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_shader.use();

        glUniformMatrix4fv(m_shader.uniform_location("view_proj"),  1, GL_FALSE, &m_camera.view_projection_matrix()[0][0]);

        glBindTextureUnit(0, m_default_texture);
        glUniform1i(m_shader.uniform_location("tex"), 0);

        m_chunk_grid->draw(time);

        // Deactivate our shader.
        m_shader.unuse();

        m_line_shader.use();

        m_phys.world->debugDrawWorld();

        f32v3 line_colour{1.0f};

        auto dims = m_process->window()->dimensions();
        std::vector<f32v3> lines;
        lines.emplace_back(f32v3{dims.width / 2.0f - 20.0f, dims.height / 2.0f,         0.0f});
        lines.emplace_back(f32v3{dims.width / 2.0f + 20.0f, dims.height / 2.0f,         0.0f});
        lines.emplace_back(f32v3{dims.width / 2.0f,         dims.height / 2.0f - 20.0f, 0.0f});
        lines.emplace_back(f32v3{dims.width / 2.0f,         dims.height / 2.0f + 20.0f, 0.0f});

        glNamedBufferSubData(
            m_crosshair_vbo, 0, lines.size() * sizeof(f32v3),
            reinterpret_cast<void*>(&lines[0])
        );

        f32m4 mvp = glm::ortho(
                        0.0f, static_cast<f32>(dims.width),
                        0.0f, static_cast<f32>(dims.height)
                    );

        glUniformMatrix4fv(m_line_shader.uniform_location("view_proj"),  1, GL_FALSE, &mvp[0][0]);
        glUniform3fv(m_line_shader.uniform_location("colour"), 1, &line_colour[0]);

        glBindVertexArray(m_crosshair_vao);
        glDrawArrays(GL_LINES, 0, 4);

        if (m_draw_chunk_outlines) {
            line_colour = {1.0f, 0.0f, 0.0f};

            glUniformMatrix4fv(m_line_shader.uniform_location("view_proj"),  1, GL_FALSE, &m_camera.view_projection_matrix()[0][0]);
            glUniform3fv(m_line_shader.uniform_location("colour"), 1, &line_colour[0]);

            m_chunk_grid->draw_grid();
        }

        m_line_shader.unuse();
    }

    virtual void init(const std::string& name, happ::ProcessBase* process) override {
        happ::ScreenBase::init(name, process);

        m_state = happ::ScreenState::RUNNING;

        m_input_manager = static_cast<happ::SingleWindowApp*>(m_process)->input_manager();
        m_input_manager->init();

        m_camera.attach_to_window(m_process->window());
        m_camera.set_position(f32v3{0.0f, 60.0f, 0.0f});
        m_camera.rotate_from_mouse_with_absolute_up(-130.0f, 160.0f, 0.005f);
        m_camera.set_fov(90.0f);
        m_camera.update();

        m_shader_cache.init(&m_iom, hg::ShaderCache::Parser{
            [](const hio::fs::path& path, hio::IOManagerBase* iom) -> std::string {
                std::string buffer;
                if (!iom->read_file_to_string(path, buffer)) return "";

                return buffer;
            }
        });

        m_shader.init(&m_shader_cache);

        m_shader.set_attribute("v_position",      0);
        m_shader.set_attribute("v_texture_coord", 1);
        m_shader.set_attribute("v_normal",        2);

        m_shader.add_shaders("shaders/test_vox.vert", "shaders/test_vox.frag");

        m_shader.link();

        m_draw_chunk_outlines = false;

        m_line_shader.init(&m_shader_cache);

        m_line_shader.set_attribute("v_position",      0);

        m_line_shader.add_shaders("shaders/line.vert", "shaders/line.frag");

        m_line_shader.link();

        m_default_texture = hg::load_texture("test_tex.png");

        {
            hthread::ThreadWorkflowBuilder workflow_builder;
            workflow_builder.init(&m_chunk_load_dag);
            workflow_builder.chain_tasks(2);
        }
        m_chunk_grid = hmem::make_handle<hvox::ChunkGrid>();
        m_chunk_grid->init(
            m_chunk_grid,
            10,
            hvox::ChunkTaskBuilder{[]() {
                return new hvox::ChunkGenerationTask<htest::voxel_screen::TVS_VoxelGenerator>();
            }}, hvox::ChunkTaskBuilder{[]() {
                return new hvox::ChunkGreedyMeshTask<htest::voxel_screen::TVS_BlockComparator>();
            }}
        );

        htest::voxel_screen::setup_physics(m_phys, m_camera, &m_line_shader);
        htest::voxel_screen::setup_player(m_player, m_phys, m_camera, m_chunk_grid);

        glCreateVertexArrays(1, &m_crosshair_vao);

        glCreateBuffers(1, &m_crosshair_vbo);
        glNamedBufferData(
            m_crosshair_vbo,
            sizeof(f32v3) * 2 * 2, // 2 points per line, 2 lines for crosshair.
            nullptr,
            GL_DYNAMIC_DRAW
        );

        glVertexArrayVertexBuffer(m_crosshair_vao, 0, m_crosshair_vbo, 0, sizeof(f32v3));

        glEnableVertexArrayAttrib(m_crosshair_vao, 0);
        glVertexArrayAttribFormat(m_crosshair_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_crosshair_vao, 0, 0);

        handle_mouse_button = hemlock::Subscriber<hui::MouseButtonEvent>{
            [&](hemlock::Sender, hui::MouseButtonEvent ev) {
                if (ev.button_id == static_cast<ui8>(hui::MouseButton::LEFT)) {
                    hvox::BlockWorldPosition position;
                    f32 distance;

                    if (hvox::Ray::cast_to_block_before(m_camera.position(), m_camera.direction(), m_chunk_grid, hvox::Block{1}, 10, position, distance)) {
                        auto chunk = m_chunk_grid->chunk(hvox::chunk_grid_position(position));

                        if (chunk != nullptr) {
                            hvox::set_block(chunk, hvox::block_chunk_position(position), hvox::Block{1});
                        }
                    }
                }
            }
        };
        hui::InputDispatcher::instance()->on_mouse.button_down += &handle_mouse_button;

        handle_mouse_move = hemlock::Subscriber<hui::MouseMoveEvent>{
            [&](hemlock::Sender, hui::MouseMoveEvent ev) {
                if (m_input_manager->is_pressed(static_cast<ui8>(hui::MouseButton::LEFT))) {
                    m_camera.rotate_from_mouse_with_absolute_up(
                        -1.0f * static_cast<f32>(ev.dx),
                        -1.0f * static_cast<f32>(ev.dy),
                        0.005f
                    );
                }
            }
        };

        hui::InputDispatcher::instance()->on_mouse.move += &handle_mouse_move;
    }
protected:
    hemlock::Subscriber<hui::MouseMoveEvent>      handle_mouse_move;
    hemlock::Subscriber<hui::MouseButtonEvent>    handle_mouse_button;

    ui32 m_default_texture;

    MyIOManager                         m_iom;
    hg::ShaderCache                     m_shader_cache;
    hcam::BasicFirstPersonCamera        m_camera;
    hui::InputManager*                  m_input_manager;
    hmem::Handle<hvox::ChunkGrid>       m_chunk_grid;
    hg::GLSLProgram                     m_shader, m_line_shader;
    hthread::ThreadWorkflowDAG          m_chunk_load_dag;
    htest::voxel_screen::PlayerData     m_player;
    htest::voxel_screen::PhysicsData    m_phys;

    bool m_draw_chunk_outlines;

    GLuint m_crosshair_vao, m_crosshair_vbo;

    std::vector<hmem::WeakHandle<hvox::Chunk>> m_unloading_chunks;
};

#endif // __hemlock_tests_test_voxel_screen_hpp

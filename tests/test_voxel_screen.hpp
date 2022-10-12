#ifndef __hemlock_tests_test_voxel_screen_hpp
#define __hemlock_tests_test_voxel_screen_hpp

#include <bullet/BulletCollision/CollisionShapes/btBoxShape.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <FastNoise/FastNoise.h>

#include "memory/handle.hpp"
#include "voxel/generation/generator_task.hpp"
#include "voxel/graphics/mesh/greedy_task.hpp"
#include "voxel/graphics/outline_renderer.hpp"
#include "voxel/ray.h"

#include "physics/voxel/chunk_grid_collider.hpp"

#include "script/environment_base.hpp"
#include "script/environment_registry.hpp"

#include "script/lua/environment.hpp"

#include "iomanager.hpp"

#if defined(DEBUG)
#   define VIEW_DIST 4
#else
#   define VIEW_DIST 10
#endif

struct TVS_BlockComparator {
    bool operator()(const hvox::Block* source, const hvox::Block* target, hvox::BlockChunkPosition, hvox::Chunk*) const {
        return (source->id == target->id) && (source->id != 0);
    }
};
struct TVS_VoxelGenerator {
    void operator() (hmem::Handle<hvox::Chunk> chunk) const {
        auto simplex_1                  = FastNoise::New<FastNoise::Simplex>();
        auto fractal_1                  = FastNoise::New<FastNoise::FractalFBm>();
        auto domain_scale_1             = FastNoise::New<FastNoise::DomainScale>();
        auto position_output_1          = FastNoise::New<FastNoise::PositionOutput>();
        auto add_1                      = FastNoise::New<FastNoise::Add>();
        auto domain_warp_grad_1         = FastNoise::New<FastNoise::DomainWarpGradient>();
        auto domain_warp_fract_prog_1   = FastNoise::New<FastNoise::DomainWarpFractalProgressive>();

        fractal_1->SetSource(simplex_1);
        fractal_1->SetOctaveCount(4);
        fractal_1->SetGain(0.5f);
        fractal_1->SetLacunarity(2.5f);

        domain_scale_1->SetSource(fractal_1);
        domain_scale_1->SetScale(0.66f);

        position_output_1->Set<FastNoise::Dim::X>(0.0f);
        position_output_1->Set<FastNoise::Dim::Y>(3.0f);
        position_output_1->Set<FastNoise::Dim::Z>(0.0f);
        position_output_1->Set<FastNoise::Dim::W>(0.0f);

        add_1->SetLHS(domain_scale_1);
        add_1->SetRHS(position_output_1);

        domain_warp_grad_1->SetSource(add_1);
        domain_warp_grad_1->SetWarpAmplitude(0.2f);
        domain_warp_grad_1->SetWarpFrequency(2.0f);

        domain_warp_fract_prog_1->SetSource(domain_warp_grad_1);
        domain_warp_fract_prog_1->SetGain(0.6f);
        domain_warp_fract_prog_1->SetOctaveCount(2);
        domain_warp_fract_prog_1->SetLacunarity(2.5f);

        f32* data = new f32[CHUNK_VOLUME];
        domain_warp_fract_prog_1->GenUniformGrid3D(
        // domain_warp_grad_1->GenUniformGrid3D(
        // fractal_1.get()->GenUniformGrid3D(
            data,
            static_cast<int>(chunk->position.x) * CHUNK_LENGTH,
            -1 * static_cast<int>(chunk->position.y) * CHUNK_LENGTH,
            static_cast<int>(chunk->position.z) * CHUNK_LENGTH,
            CHUNK_LENGTH,
            CHUNK_LENGTH,
            CHUNK_LENGTH,
            0.005f,
            1337
        );
        // f32* data = new f32[CHUNK_AREA];
        // domain_warp_fract_prog_1->GenUniformGrid2D(
        // // domain_warp_grad_1->GenUniformGrid2D(
        // // fractal_1.get()->GenUniformGrid2D(
        //     data,
        //     static_cast<int>(chunk->position.x) * CHUNK_LENGTH,
        //     static_cast<int>(chunk->position.z) * CHUNK_LENGTH,
        //     CHUNK_LENGTH,
        //     CHUNK_LENGTH,
        //     0.2f,
        //     1337
        // );

        {
            std::lock_guard lock(chunk->blocks_mutex);

            ui64 noise_idx = 0;
            for (ui8 z = 0; z < CHUNK_LENGTH; ++z) {
                for (ui8 y = 0; y < CHUNK_LENGTH; ++y) {
                    for (ui8 x = 0; x < CHUNK_LENGTH; ++x) {
                        chunk->blocks[hvox::block_index({x, CHUNK_LENGTH - y - 1, z})] = data[noise_idx++] > 0 ? hvox::Block{1} : hvox::Block{0};
                    }
                }
            }
        }

        // hvox::set_blocks(chunk, hvox::BlockChunkPosition{0}, hvox::BlockChunkPosition{CHUNK_LENGTH - 1}, hvox::Block{0});
        // ui64 noise_idx = 0;
        // for (ui8 z = 0; z < CHUNK_LENGTH; ++z) {
        //     for (ui8 x = 0; x < CHUNK_LENGTH; ++x) {
        //         i32 y = hvox::block_world_position(chunk->position, 0).y;

        //         i32 height = -1 * static_cast<i32>(data[noise_idx++]);
        //         // debug_printf("Height at (%d, %d): %d\n", x, z, height);
        //         if (y >= height) continue;

        //         hvox::set_blocks(chunk, {x, 0, z}, {x, glm::min(height - y, CHUNK_LENGTH - 1), z}, hvox::Block{1});
        //     }
        // }

        delete[] data;
    }
};

struct TVS_VoxelShapeEvaluator {
    btCollisionShape* operator()(hvox::Block b, btTransform&) const {
        if (b == hvox::Block{1}) {
            return new btBoxShape(btVector3{0.5f, 0.5f, 0.5f});
        }
        return nullptr;
    }
};

struct TVS_ChunkOutlinePredicate {
    std::tuple<bool, colour4> operator()(hmem::Handle<hvox::Chunk>) {
        return {
            true, {255, 0, 0, 255}
        };
    }
};

class VoxelPhysDrawer : public btIDebugDraw {
public:
    VoxelPhysDrawer(hcam::BasicFirstPersonCamera* camera, hg::GLSLProgram* shader) : btIDebugDraw() {
        m_camera = camera;
        m_line_shader = shader;

        glCreateVertexArrays(1, &m_vao);

        glCreateBuffers(1, &m_vbo);
        glNamedBufferData(
            m_vbo,
            sizeof(f32v3) * 2,
            nullptr,
            GL_DYNAMIC_DRAW
        );

        glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(f32v3));

        glEnableVertexArrayAttrib(m_vao, 0);
        glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_vao, 0, 0);
    }
    virtual ~VoxelPhysDrawer() { /* Empty. */ }

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3&) override {
        f32v3 points[2] = {
            f32v3{
                from.x()/* + m_camera->position().x*/,
                from.y()/* + m_camera->position().y*/,
                from.z()/* + m_camera->position().z*/
            },
            f32v3{
                to.x()/* + m_camera->position().x*/,
                to.y()/* + m_camera->position().y*/,
                to.z()/* + m_camera->position().z*/
            }
        };

        glNamedBufferSubData(m_vbo, 0, 2 * sizeof(f32v3), reinterpret_cast<void*>(&points[0]));

        f32v3 _colour = f32v3{1.0f, 1.0f, 0.0f};
        glUniformMatrix4fv(m_line_shader->uniform_location("view_proj"),  1, GL_FALSE, &m_camera->view_projection_matrix()[0][0]);
        glUniform3fv(m_line_shader->uniform_location("colour"), 1, &_colour[0]);

        glBindVertexArray(m_vao);
        glDrawArrays(GL_LINES, 0, 2);
    }

    virtual void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) override { /* Empty. */ }

    virtual void reportErrorWarning(const char* warning_string [[maybe_unused]]) override { debug_printf(warning_string); }

    virtual void draw3dText(const btVector3&, const char*) override { /* Empty. */ }

    virtual void setDebugMode(int debug_mode) override { m_debug_mode = debug_mode; }

    virtual int getDebugMode() const override { return m_debug_mode; }
protected:
    int m_debug_mode;
    GLuint m_vao, m_vbo;
    hcam::BasicFirstPersonCamera* m_camera;
    hg::GLSLProgram* m_line_shader;
};


class TestVoxelScreen : public happ::ScreenBase {
public:
    TestVoxelScreen() :
        happ::ScreenBase(),
        m_input_manager(nullptr)
    { /* Empty. */ }
    virtual ~TestVoxelScreen() { /* Empty */ };

    virtual void start(hemlock::FrameTime time) override {
        happ::ScreenBase::start(time);

        for (auto x = -VIEW_DIST; x <= VIEW_DIST; ++x) {
            for (auto z = -VIEW_DIST; z <= VIEW_DIST; ++z) {
                for (auto y = -2; y < 6; ++ y) {
                    m_chunk_grid->preload_chunk_at({ {x, y, z} });
                }
            }
        }
        for (auto x = -VIEW_DIST; x <= VIEW_DIST; ++x) {
            for (auto z = -VIEW_DIST; z <= VIEW_DIST; ++z) {
                for (auto y = -2; y < 6; ++ y) {
                    m_chunk_grid->load_chunk_at({ {x, y, z} });
                }
            }
        }
    }

    virtual void update(hemlock::FrameTime time) override {
        static f32v3 last_pos{0.0f};

        if (m_input_manager->is_pressed(hui::PhysicalKey::H_G)) {
            m_phys.world->setGravity(btVector3(0, -9.8f, 0));
            debug_printf("Turning on gravity.\n");
        }

        static bool do_chunk_check = false;

        if (m_input_manager->is_pressed(hui::PhysicalKey::H_J)) {
            do_chunk_check = false;
        }

        if (do_chunk_check) return;

        if (m_input_manager->is_pressed(hui::PhysicalKey::H_K)) {
            do_chunk_check = true;
        }

        if (m_input_manager->is_pressed(hui::PhysicalKey::H_L)) {
            m_draw_chunk_outlines = !m_draw_chunk_outlines;
        }

        f32 speed_mult = 1.0f;
        if (m_input_manager->key_modifier_state().ctrl) {
            speed_mult = 10.0f;
        }
        if (m_input_manager->key_modifier_state().alt) {
            speed_mult = 50.0f;
        }

        f32 frame_time = hemlock::frame_time_to_floating<>(time);

        f32v3 delta_pos{0.0f};
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_W)) {
            delta_pos += glm::normalize(m_camera.direction()) * frame_time * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_A)) {
            delta_pos -= glm::normalize(m_camera.right()) * frame_time * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_S)) {
            delta_pos -= glm::normalize(m_camera.direction()) * frame_time * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_D)) {
            delta_pos += glm::normalize(m_camera.right()) * frame_time * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_Q)) {
            delta_pos += glm::normalize(m_camera.up()) * frame_time * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_E)) {
            delta_pos -= glm::normalize(m_camera.up()) * frame_time * 0.01f * speed_mult;
        }

        static bool do_unloads = false;
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_U)) {
            do_unloads = true;
        }
        if (do_unloads) {
            static f32 t = 0.0f;
            t += hemlock::frame_time_to_floating<>(time);
            for (auto x = -VIEW_DIST; x < VIEW_DIST; ++x) {
                for (auto z = -VIEW_DIST; z < VIEW_DIST; ++z) {
                    if (((x + VIEW_DIST) + (2 * VIEW_DIST + 1) * (z + VIEW_DIST)) * 300 < t) {
                        m_chunk_grid->unload_chunk_at({ {x, 0, z} });
                        m_chunk_grid->unload_chunk_at({ {x, 1, z} });
                        m_chunk_grid->unload_chunk_at({ {x, 2, z} });
                    }
                }
            }
        }

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

        m_player.ac.position += hvox::EntityWorldPosition{f32v3{delta_pos.x, delta_pos.y, delta_pos.z} * static_cast<f32>(1ll << 32)};
        m_camera.offset_position(f32v3{delta_pos.x, delta_pos.y, delta_pos.z});
        {
            auto transform = m_player_body->getWorldTransform();
            transform.setOrigin(btVector3(m_camera.position().x, m_camera.position().y, m_camera.position().z));
            m_player_body->setWorldTransform(transform);
        }
        m_camera.update();

        f32v3 current_pos = glm::floor(m_camera.position() / static_cast<f32>(CHUNK_LENGTH));

        i32 x_step = static_cast<i32>(current_pos.x) - static_cast<i32>(last_pos.x);
        if (x_step != 0) {
            for (auto z = static_cast<i32>(current_pos.z) - VIEW_DIST; z <= static_cast<i32>(current_pos.z) + VIEW_DIST; ++z) {
                for (auto y = -2; y < 6; ++ y) {
                    m_unloading_chunks.emplace_back(hmem::WeakHandle<hvox::Chunk>{});
                    auto& handle = m_unloading_chunks.back();
                    m_chunk_grid->unload_chunk_at({ {
                        x_step < 0 ? static_cast<i32>(current_pos.x) + VIEW_DIST + 1 : static_cast<i32>(current_pos.x) - VIEW_DIST - 1,
                        y, z
                    } }, &handle);
                }
            }
            for (auto z = static_cast<i32>(current_pos.z) - VIEW_DIST; z <= static_cast<i32>(current_pos.z) + VIEW_DIST; ++z) {
                for (auto y = -2; y < 6; ++ y) {
                    m_chunk_grid->preload_chunk_at({ {
                        x_step < 0 ? static_cast<i32>(current_pos.x) - VIEW_DIST : static_cast<i32>(current_pos.x) + VIEW_DIST,
                        y, z
                    } });
                }
            }
            for (auto z = static_cast<i32>(current_pos.z) - VIEW_DIST; z <= static_cast<i32>(current_pos.z) + VIEW_DIST; ++z) {
                for (auto y = -2; y < 6; ++ y) {
                    m_chunk_grid->load_chunk_at({ {
                        x_step < 0 ? static_cast<i32>(current_pos.x) - VIEW_DIST : static_cast<i32>(current_pos.x) + VIEW_DIST,
                        y, z
                    } });
                }
            }
        }

        i32 z_step = static_cast<i32>(current_pos.z) - static_cast<i32>(last_pos.z);
        if (z_step != 0) {
            for (auto x = static_cast<i32>(current_pos.x) - VIEW_DIST; x <= static_cast<i32>(current_pos.x) + VIEW_DIST; ++x) {
                for (auto y = -2; y < 6; ++ y) {
                    m_unloading_chunks.emplace_back(hmem::WeakHandle<hvox::Chunk>{});
                    auto& handle = m_unloading_chunks.back();
                    m_chunk_grid->unload_chunk_at({ {
                        x, y,
                        z_step < 0 ? static_cast<i32>(current_pos.z) + VIEW_DIST + 1 : static_cast<i32>(current_pos.z) - VIEW_DIST - 1
                    } }, &handle);
                }
            }
            for (auto x = static_cast<i32>(current_pos.x) - VIEW_DIST; x <= static_cast<i32>(current_pos.x) + VIEW_DIST; ++x) {
                for (auto y = -2; y < 6; ++ y) {
                    m_chunk_grid->preload_chunk_at({ {
                        x, y,
                        z_step < 0 ? static_cast<i32>(current_pos.z) - VIEW_DIST : static_cast<i32>(current_pos.z) + VIEW_DIST
                    } });
                }
            }
            for (auto x = static_cast<i32>(current_pos.x) - VIEW_DIST; x <= static_cast<i32>(current_pos.x) + VIEW_DIST; ++x) {
                for (auto y = -2; y < 6; ++ y) {
                    m_chunk_grid->load_chunk_at({ {
                        x, y,
                        z_step < 0 ? static_cast<i32>(current_pos.z) - VIEW_DIST : static_cast<i32>(current_pos.z) + VIEW_DIST
                    } });
                }
            }
        }

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
        if (hphys::ChunkGridCollider::determine_candidate_colliding_voxels<TVS_VoxelShapeEvaluator>(m_player.ac, m_player.dc, m_player.cc, voxel_patch)) {
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

        m_player_body->activate();
        m_phys.world->stepSimulation(hemlock::frame_time_to_floating<std::chrono::seconds, btScalar>(time));

        m_camera.set_position(f32v3{
            m_player_body->getWorldTransform().getOrigin().x(),
            m_player_body->getWorldTransform().getOrigin().y(),
            m_player_body->getWorldTransform().getOrigin().z()
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
            if (hphys::ChunkGridCollider::determine_candidate_colliding_voxels<TVS_VoxelShapeEvaluator>(m_player.ac, m_player.dc, m_player.cc, _voxel_patch)) {
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

        m_line_shader.unuse();

        if (m_draw_chunk_outlines) {
            m_chunk_outline_shader.use();

            glUniformMatrix4fv(m_chunk_outline_shader.uniform_location("view_proj"), 1, GL_FALSE, &m_camera.view_projection_matrix()[0][0]);

            m_outline_renderer.draw(time);

            m_chunk_outline_shader.unuse();
        }

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
        m_chunk_outline_shader.init(&m_shader_cache);
        m_chunk_outline_shader.add_shaders("shaders/chunk_outline.vert", "shaders/chunk_outline.frag");
        m_chunk_outline_shader.link();

        m_default_texture = hg::load_texture("test_tex.png");

        {
            hthread::ThreadWorkflowBuilder workflow_builder;
            workflow_builder.init(&m_chunk_load_dag);
            workflow_builder.chain_tasks(2);
        }
        m_chunk_grid = hmem::make_handle<hvox::ChunkGrid>();
        m_chunk_grid->init(
            m_chunk_grid,
            VIEW_DIST * 2 + 1,
            16,
            hvox::ChunkTaskBuilder{[]() {
                return new hvox::ChunkGenerationTask<TVS_VoxelGenerator>();
            }}, hvox::ChunkTaskBuilder{[]() {
                return new hvox::ChunkGreedyMeshTask<TVS_BlockComparator>();
            }}
        );

        m_outline_renderer.init(TVS_ChunkOutlinePredicate{}, m_chunk_grid);

        m_player.ac.position   = hvox::EntityWorldPosition{0, static_cast<hvox::EntityWorldPositionCoord>(60) << 32, 0};
        m_player.ac.chunk_grid = m_chunk_grid;
        m_player.cc.shape = new btCompoundShape();
        m_player.cc.shape->addChildShape(btTransform::getIdentity(), new btBoxShape(btVector3{0.5f, 1.5f, 0.5f}));
        // TODO(Matthew): update this.
        m_player.dc.velocity   = f32v3(2.0f);

        m_phys.broadphase       = new btDbvtBroadphase();
        m_phys.collision_config = new btDefaultCollisionConfiguration();
        m_phys.dispatcher       = new btCollisionDispatcher(m_phys.collision_config);
        m_phys.solver           = new btSequentialImpulseConstraintSolver();
        m_phys.world            = new btDiscreteDynamicsWorld(m_phys.dispatcher, m_phys.broadphase, m_phys.solver, m_phys.collision_config);
        m_phys.world->setGravity(btVector3(0, 0, 0));
        m_phys.world->setDebugDrawer(new VoxelPhysDrawer(&m_camera, &m_line_shader));
        m_phys.world->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);

        {
            btQuaternion rotation;
            rotation.setEulerZYX(0.0f, 1.0f, 0.0f);
            btVector3 position = btVector3(m_camera.position().x, m_camera.position().y, m_camera.position().z);
            btDefaultMotionState* motion_state = new btDefaultMotionState(btTransform(rotation, position));
            btVector3 inertia;
            btScalar mass = 80.0f;
            m_player.cc.shape->calculateLocalInertia(mass, inertia);
            btRigidBody::btRigidBodyConstructionInfo body_info = btRigidBody::btRigidBodyConstructionInfo(mass, motion_state, m_player.cc.shape, inertia);
            body_info.m_restitution = 0.0f;
            body_info.m_friction = 1000.0f;
            m_player_body = new btRigidBody(body_info);
            m_player_body->setAngularFactor(0.0f);
            m_phys.world->addRigidBody(m_player_body);
        }

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

    MyIOManager                     m_iom;
    hg::ShaderCache                 m_shader_cache;
    hcam::BasicFirstPersonCamera    m_camera;
    hui::InputManager*              m_input_manager;
    hmem::Handle<hvox::ChunkGrid>   m_chunk_grid;
    hg::GLSLProgram                 m_shader, m_line_shader, m_chunk_outline_shader;
    hthread::ThreadWorkflowDAG      m_chunk_load_dag;
    struct {
        hphys::CollidableComponent cc;
        hphys::AnchoredComponent   ac;
        hphys::DynamicComponent    dc;
    } m_player;
    btRigidBody* m_player_body;
    struct {
        btDbvtBroadphase*                       broadphase;
        btDefaultCollisionConfiguration*        collision_config;
        btCollisionDispatcher*                  dispatcher;
        btSequentialImpulseConstraintSolver*    solver;
        btDiscreteDynamicsWorld*                world;
    } m_phys;

    hvox::ConditionalChunkOutlineRenderer<TVS_ChunkOutlinePredicate> m_outline_renderer;
    bool m_draw_chunk_outlines;

    GLuint m_crosshair_vao, m_crosshair_vbo;

    std::vector<hmem::WeakHandle<hvox::Chunk>> m_unloading_chunks;
};

#endif // __hemlock_tests_test_voxel_screen_hpp

#ifndef __hemlock_tests_test_render_screen_hpp
#define __hemlock_tests_test_render_screen_hpp

#include "app/screen_base.h"
#include "camera/basic_first_person_camera.h"
#include "graphics/font/font.h"
#include "graphics/glsl_program.h"
#include "graphics/sprite/batcher.h"
#include "graphics/texture.hpp"
#include "memory/handle.hpp"
#include "ui/input/dispatcher.h"
#include "ui/input/keys.hpp"
#include "ui/input/manager.h"
#include "voxel/chunk/grid.h"
#include "voxel/generation/generator_task.hpp"
#include "voxel/graphics/mesh/greedy_strategy.hpp"
#include "voxel/graphics/mesh/mesh_task.hpp"

#include "iomanager.hpp"

struct TRS_VoxelComparator {
    bool
    operator()(hvox::Voxel source, hvox::Voxel target, hvox::VoxelChunkPosition, hvox::Chunk*)
        const {
        return (source == target) && (source != hvox::NULL_VOXEL);
    }
};

struct TRS_VoxelGenerator {
    void operator()(hmem::Handle<hvox::Chunk> chunk) const {
        set_voxels(
            chunk,
            hvox::VoxelChunkPosition{ 0 },
            hvox::VoxelChunkPosition{ CHUNK_LENGTH - 1 },
            hvox::Voxel{ 0 }
        );

        // for (auto y = 0; y < CHUNK_LENGTH; y += 2) {
        // for (auto x = 0; x < CHUNK_LENGTH; x += 2) {
        for (auto y = 0; y < 10; y += 1) {
            const ui8 LEFT  = 14;
            const ui8 RIGHT = 16;

            // set_voxels(chunk, hvox::VoxelChunkPosition{0, y, 0},
            // hvox::VoxelChunkPosition{LEFT, y, LEFT}, hvox::Voxel{1});
            // set_voxels(chunk, hvox::VoxelChunkPosition{RIGHT, y, RIGHT},
            // hvox::VoxelChunkPosition{CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1},
            // hvox::Voxel{1});

            // set_voxels(chunk, hvox::VoxelChunkPosition{x, 0, 0},
            // hvox::VoxelChunkPosition{x, LEFT, LEFT}, hvox::Voxel{1});
            // set_voxels(chunk, hvox::VoxelChunkPosition{x, RIGHT, RIGHT},
            // hvox::VoxelChunkPosition{x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
            // hvox::Voxel{1});

            set_voxels(
                chunk,
                hvox::VoxelChunkPosition{ 0, y, 0 },
                hvox::VoxelChunkPosition{ LEFT - y, y, LEFT - y },
                hvox::Voxel{ 1 }
            );
            set_voxels(
                chunk,
                hvox::VoxelChunkPosition{ RIGHT + y, y, RIGHT + y },
                hvox::VoxelChunkPosition{ CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1 },
                hvox::Voxel{ 1 }
            );
        }
    }
};

class TestRenderScreen : public happ::ScreenBase {
public:
    TestRenderScreen() : happ::ScreenBase(), m_input_manager(nullptr) { /* Empty. */
    }

    virtual ~TestRenderScreen(){
        /* Empty. */
    };

    virtual void dispose() override {
        auto font = m_font_cache.fetch("fonts/Orbitron-Regular.ttf");
        font->dispose();

        m_chunk_grid->dispose();

        happ::ScreenBase::dispose();
    }

    virtual void start(hemlock::FrameTime time) override {
        happ::ScreenBase::start(time);

#define NUM 6
        for (auto x = -NUM; x < NUM; ++x) {
            for (auto z = -NUM; z < NUM; ++z) {
                for (auto y = -2 * NUM; y < 0; ++y) {
                    m_chunk_grid->preload_chunk_at({
                        {x, y, z}
                    });
                }
            }
        }
        for (auto x = -NUM; x < NUM; ++x) {
            for (auto z = -NUM; z < NUM; ++z) {
                for (auto y = -2 * NUM; y < 0; ++y) {
                    m_chunk_grid->load_chunk_at({
                        {x, y, z}
                    });
                }
            }
        }
#undef NUM
    }

    virtual void update(hemlock::FrameTime time) override {
        m_sprite_batcher.begin();
        m_sprite_batcher.add_sprite(
            f32v2{
                60.0f
                    + 30.0f
                          * sin(std::chrono::duration_cast<std::chrono::seconds>(time)
                                    .count()),
                60.0f
                    + 30.0f
                          * cos(std::chrono::duration_cast<std::chrono::seconds>(time)
                                    .count()) },
            f32v2{ 200.0f, 200.0f },
            colour4{ 255, 0, 0, 255 },
            colour4{ 0, 255, 0, 255 },
            hg::Gradient::LEFT_TO_RIGHT
        );
        m_sprite_batcher.add_string(
            "Hello, world!",
            f32v4{ 300.0f, 300.0f, 1000.0f, 1000.0f },
            f32v4{ 295.0f, 295.0f, 1010.0f, 1010.0f },
            hg::f::StringSizing{ hg::f::StringSizingKind::SCALED, { f32v2{ 1.0f } } },
            colour4{ 0, 0, 0, 255 },
            "fonts/Orbitron-Regular.ttf",
            hg::f::TextAlign::TOP_LEFT,
            hg::f::WordWrap::NONE  //,
            // 0.0f,
            // hg::f::FontStyle::NORMAL,
            // hg::f::FontRenderStyle::SOLID
        );
        m_sprite_batcher.end();

        m_chunk_grid->update(time);

        f32 speed_mult = 1.0f;
        if (m_input_manager->key_modifier_state().ctrl) {
            speed_mult = 10.0f;
        }
        if (m_input_manager->key_modifier_state().alt) {
            speed_mult = 50.0f;
        }

        f32 frame_time = hemlock::frame_time_to_floating<>(time);

        f32v3 delta_pos{ 0.0f };
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_W)) {
            delta_pos += glm::normalize(m_camera.direction()) * frame_time * 0.01f
                         * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_A)) {
            delta_pos
                -= glm::normalize(m_camera.right()) * frame_time * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_S)) {
            delta_pos -= glm::normalize(m_camera.direction()) * frame_time * 0.01f
                         * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_D)) {
            delta_pos
                += glm::normalize(m_camera.right()) * frame_time * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_Q)) {
            delta_pos
                += glm::normalize(m_camera.up()) * frame_time * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_E)) {
            delta_pos
                -= glm::normalize(m_camera.up()) * frame_time * 0.01f * speed_mult;
        }

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

        // const f32 turn_clamp_on_at = 60.0f / 360.0f * 2.0f * M_PI;
        // f32 up_angle = glm::acos(glm::dot(m_camera.up(), hcam::ABSOLUTE_UP));
        // if (up_angle < -1.0f * turn_clamp_on_at || up_angle > turn_clamp_on_at)
        //     m_camera.set_clamp_enabled(true);

        m_camera.offset_position(delta_pos);
        m_camera.update();
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

#if !defined(HEMLOCK_OS_MAC)
        glBindTextureUnit(0, m_default_texture);

        glUniform1i(m_shader.uniform_location("tex"), 0);
#else   // !defined(HEMLOCK_OS_MAC)
        glBindTexture(GL_TEXTURE_2D, m_default_texture);
#endif  // !defined(HEMLOCK_OS_MAC)

        m_chunk_grid->draw(time);

        // Deactivate our shader.
        m_shader.unuse();

        // happ::WindowDimensions dims = m_process->window()->dimensions();
        // m_sprite_batcher.render(f32v2{dims.width, dims.height});
        m_sprite_batcher.render(f32m4{ 1.0f }, m_camera.view_projection_matrix());
    }

    virtual void init(const std::string& name, happ::ProcessBase* process) override {
        happ::ScreenBase::init(name, process);

        m_state = happ::ScreenState::RUNNING;

        m_input_manager
            = static_cast<happ::SingleWindowApp*>(m_process)->input_manager();

        m_camera.attach_to_window(m_process->window());
        m_camera.set_position(f32v3{ 270.0f, 230.0f, -470.0f });
        m_camera.rotate_from_mouse_with_absolute_up(-110.0f, 110.0f, 0.005f);
        m_camera.set_fov(90.0f);
        // m_camera.set_clamp({false, 30.0f / 360.0f * 2.0f * M_PI});
        m_camera.update();

        // std::cout << std::endl << my_shader_parser("shaders/default_sprite.frag",
        // &my_iom) << std::endl << std::endl;
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

        m_default_texture = hg::load_texture("test_tex.png");

        {
            hthread::ThreadWorkflowBuilder workflow_builder;
            workflow_builder.init(&m_chunk_load_dag);
            workflow_builder.chain_tasks(2);
        }
        m_chunk_grid = hmem::make_handle<hvox::ChunkGrid>();
        m_chunk_grid->init(
            m_chunk_grid,
            6 * 2 + 1,
            10,
            hvox::ChunkTaskBuilder{ []() {
                return new hvox::ChunkGenerationTask<TRS_VoxelGenerator>();
            } },
            hvox::ChunkTaskBuilder{ []() {
                return new hvox::ChunkMeshTask<
                    hvox::GreedyMeshStrategy<TRS_VoxelComparator>>();
            } }
        );

        handle_mouse_move = hemlock::Subscriber<hui::MouseMoveEvent>{
            [&](hemlock::Sender, hui::MouseMoveEvent ev) {
                if (m_input_manager->is_pressed(static_cast<ui8>(hui::MouseButton::LEFT)
                    )) {
                    m_camera.rotate_from_mouse_with_absolute_up(
                        -1.0f * static_cast<f32>(ev.dx),
                        -1.0f * static_cast<f32>(ev.dy),
                        0.005f
                    );
                }
            }
        };

        hui::InputDispatcher::instance()->on_mouse.move += &handle_mouse_move;

        m_font_cache.init(
            &m_iom,
            hg::f::FontCache::Parser{
                [](const hio::fs::path& path, hio::IOManagerBase* iom) -> hg::f::Font {
                    hio::fs::path actual_path;
                    if (!iom->resolve_path(path, actual_path)) return hg::f::Font{};

                    hg::f::Font font;
                    font.init(actual_path.string());

                    return font;
                } }
        );

        auto font = m_font_cache.fetch("fonts/Orbitron-Regular.ttf");
        font->set_default_size(50);
        font->generate(/*hg::f::FontStyle::NORMAL, hg::f::FontRenderStyle::SOLID*/);
        // auto font_instance = font->get_instance(/*hg::f::FontStyle::NORMAL,
        // hg::f::FontRenderStyle::SOLID*/); font_instance.save("test.png",
        // {hio::img::png::save});

        m_sprite_batcher.init(&m_shader_cache, &m_font_cache);
    }
protected:
    hemlock::Subscriber<hui::MouseMoveEvent> handle_mouse_move;

    ui32 m_default_texture;

    MyIOManager                   m_iom;
    hg::ShaderCache               m_shader_cache;
    hg::f::FontCache              m_font_cache;
    hg::s::SpriteBatcher          m_sprite_batcher;
    hcam::BasicFirstPersonCamera  m_camera;
    hui::InputManager*            m_input_manager;
    hmem::Handle<hvox::ChunkGrid> m_chunk_grid;
    hg::GLSLProgram               m_shader;
    hthread::ThreadWorkflowDAG    m_chunk_load_dag;
};

#endif  // __hemlock_tests_test_render_screen_hpp

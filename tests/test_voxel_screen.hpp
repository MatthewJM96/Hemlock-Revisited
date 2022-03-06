#ifndef __hemlock_tests_test_voxel_screen_hpp
#define __hemlock_tests_test_voxel_screen_hpp

#include <FastNoise/FastNoise.h>

#include "voxel/chunk/generator_task.h"
#include "voxel/chunk/mesh/naive_task.h"

#include "iomanager.hpp"

struct TVS_BlockComparator {
    bool operator()(const hvox::Block* source, const hvox::Block* target, hvox::BlockChunkPosition, hvox::Chunk*) const {
        return (source->id == target->id) && (source->id != 0);
    }
};
struct TVS_VoxelGenerator {
    void operator() (hvox::Chunk* chunk) const {
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

        f32* data = new f32[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
        domain_warp_fract_prog_1->GenUniformGrid3D(
            data,
            chunk->position.x * CHUNK_SIZE,
            -1 * chunk->position.y * CHUNK_SIZE,
            chunk->position.z * CHUNK_SIZE,
            CHUNK_SIZE,
            CHUNK_SIZE,
            CHUNK_SIZE,
            0.005f,
            1337
        );

        ui64 noise_idx = 0;
        for (ui8 z = 0; z < CHUNK_SIZE; ++z) {
            for (ui8 y = 0; y < CHUNK_SIZE; ++y) {
                for (ui8 x = 0; x < CHUNK_SIZE; ++x) {
                    chunk->blocks[hvox::block_index({x, CHUNK_SIZE - y - 1, z})] = data[noise_idx++] > 0 ? hvox::Block{1} : hvox::Block{0};
                }
            }
        }

        delete[] data;
    }
};


class TestVoxelScreen : public happ::ScreenBase {
public:
    TestVoxelScreen() :
        happ::ScreenBase(),
        m_input_manager(nullptr)
    { /* Empty. */ }
    virtual ~TestVoxelScreen() { /* Empty */ };

    virtual void start(TimeData time) override {
        happ::ScreenBase::start(time);

#define NUM 4
        for (auto x = -NUM; x < NUM; ++x) {
            for (auto z = -NUM; z < NUM; ++z) {
                for (auto y = -1; y < 2; ++ y) {
                    m_chunk_grid.preload_chunk_at({ x, y, z });
                }
            }
        }
        for (auto x = -NUM; x < NUM; ++x) {
            for (auto z = -NUM; z < NUM; ++z) {
                for (auto y = -1; y < 2; ++ y) {
                    m_chunk_grid.load_chunk_at({ x, y, z });
                }
            }
        }
#undef NUM
    }

    virtual void update(TimeData time) override {
        m_chunk_grid.update(time);

        f32 speed_mult = 1.0f;
        if (m_input_manager->key_modifier_state().ctrl) {
            speed_mult = 10.0f;
        }
        if (m_input_manager->key_modifier_state().alt) {
            speed_mult = 50.0f;
        }

        f32v3 delta_pos{0.0f};
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_W)) {
            delta_pos += glm::normalize(m_camera.direction()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_A)) {
            delta_pos -= glm::normalize(m_camera.right()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_S)) {
            delta_pos -= glm::normalize(m_camera.direction()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_D)) {
            delta_pos += glm::normalize(m_camera.right()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_Q)) {
            delta_pos += glm::normalize(m_camera.up()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_E)) {
            delta_pos -= glm::normalize(m_camera.up()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }

#if defined(DEBUG)
        static f64 last_time = 0.0;
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_T)) {
            if (last_time + 1000.0 < time.total) {
                last_time = time.total;
                f32v3 pos = m_camera.position();
                f32v3 dir = m_camera.direction();
                debug_printf("Camera Coords: (%f, %f, %f)\nCamera Direction: (%f, %f, %f)\n", pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);
            }
        }
#endif

        m_camera.offset_position(delta_pos);
        m_camera.update();
    }
    virtual void draw(TimeData time) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_shader.use();

        glUniformMatrix4fv(m_shader.uniform_location("view_proj"),  1, GL_FALSE, &m_camera.view_projection_matrix()[0][0]);

        glBindTextureUnit(0, m_default_texture);
        glUniform1i(m_shader.uniform_location("tex"), 0);

        m_chunk_grid.draw(time);

        // Deactivate our shader.
        m_shader.unuse();
    }

    virtual void init(const std::string& name, happ::ProcessBase* process) override {
        happ::ScreenBase::init(name, process);

        m_state = happ::ScreenState::RUNNING;

        m_input_manager = static_cast<happ::SingleWindowApp*>(m_process)->input_manager();

        m_camera.attach_to_window(m_process->window());
        m_camera.set_position(f32v3{35.0f, 9.0f, -22.0f});
        m_camera.rotate_from_mouse_with_absolute_up(-160.0f, 160.0f, 0.005f);
        m_camera.set_fov(90.0f);
        m_camera.update();

        m_shader_cache.init(&m_iom, hg::ShaderCache::Parser(
            [](const hio::fs::path& path, hio::IOManagerBase* iom) -> std::string {
                std::string buffer;
                if (!iom->read_file_to_string(path, buffer)) return "";

                return buffer;
            }
        ));

        m_shader.init(&m_shader_cache);

        m_shader.set_attribute("v_position",      0);
        m_shader.set_attribute("v_texture_coord", 1);

        m_shader.add_shaders("shaders/test_vox.vert", "shaders/test_vox.frag");

        m_shader.link();

        m_default_texture = hg::load_texture("test_tex.png");

        {
            hthread::ThreadWorkflowBuilder workflow_builder;
            workflow_builder.init(&m_chunk_load_dag);
            workflow_builder.chain_tasks(2);
        }
        m_chunk_grid.init(10, &m_chunk_load_dag, hvox::ChunkLoadTaskListBuilder([](hvox::Chunk* chunk, hvox::ChunkGrid* chunk_grid) {
            // TODO(Matthew): How do we clean up this?
            hthread::HeldWorkflowTask<hvox::ChunkLoadTaskContext>* tasks = new hthread::HeldWorkflowTask<hvox::ChunkLoadTaskContext>[2];

            auto gen_task  = new hvox::ChunkGenerationTask<TVS_VoxelGenerator>();
            // Greedy meshing is broken.
            // auto mesh_task = new hvox::ChunkGreedyMeshTask<TRS_BlockComparator>();
            auto mesh_task = new hvox::ChunkNaiveMeshTask<TRS_BlockComparator>();

            gen_task->init(chunk, chunk_grid);
            mesh_task->init(chunk, chunk_grid);

            tasks[0] = { reinterpret_cast<hthread::IThreadWorkflowTask<hvox::ChunkLoadTaskContext>*>(gen_task),  true };
            tasks[1] = { reinterpret_cast<hthread::IThreadWorkflowTask<hvox::ChunkLoadTaskContext>*>(mesh_task), true };

            return hthread::ThreadWorkflowTasksView<hvox::ChunkLoadTaskContext>{ tasks, 2 };
        }));

        handle_mouse_move = hemlock::Subscriber<hui::MouseMoveEvent>(
            [&](hemlock::Sender, hui::MouseMoveEvent ev) {
                if (m_input_manager->is_pressed(static_cast<ui8>(hui::MouseButton::LEFT))) {
                    m_camera.rotate_from_mouse_with_absolute_up(
                        -1.0f * static_cast<f32>(ev.dx),
                        -1.0f * static_cast<f32>(ev.dy),
                        0.005f
                    );
                }
            }
        );

        hui::InputDispatcher::instance()->on_mouse.move += &handle_mouse_move;
    }
protected:
    hemlock::Subscriber<hui::MouseMoveEvent>      handle_mouse_move;

    ui32 m_default_texture;

    MyIOManager                  m_iom;
    hg::ShaderCache              m_shader_cache;
    hcam::BasicFirstPersonCamera m_camera;
    hui::InputManager*           m_input_manager;
    hvox::ChunkGrid              m_chunk_grid;
    hg::GLSLProgram              m_shader;
    hthread::ThreadWorkflowDAG   m_chunk_load_dag;
};

#endif // __hemlock_tests_test_voxel_screen_hpp

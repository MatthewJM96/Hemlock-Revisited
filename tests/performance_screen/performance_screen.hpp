#ifndef __hemlock_tests_test_performance_screen_hpp
#define __hemlock_tests_test_performance_screen_hpp

#include <FastNoise/FastNoise.h>

#include "rand.h"
#include "memory/handle.hpp"
#include "graphics/font/font.h"
#include "graphics/glsl_program.h"
#include "graphics/sprite/batcher.h"
#include "ui/input/dispatcher.h"
#include "voxel/ai/navmesh_task.hpp"
#include "voxel/generation/generator_task.hpp"
#include "voxel/graphics/mesh/mesh_task.hpp"
#include "voxel/graphics/mesh/greedy_strategy.hpp"
#include "voxel/graphics/mesh/instance_manager.h"

#include "tests/iomanager.hpp"

#include "tests/performance_screen/terrain.hpp"

class TestPerformanceScreen : public happ::ScreenBase {
public:
    TestPerformanceScreen() : happ::ScreenBase(), m_do_gen_profile(false), m_do_naive_mesh_profile(false), m_do_greedy_mesh_profile(false), m_do_navmesh_profile(false) { /* Empty. */
    }

    virtual ~TestPerformanceScreen(){ /* Empty */ };

    virtual void start(hemlock::FrameTime time) override {
        happ::ScreenBase::start(time);

        const ui32 xyz_len    = 18;
        const ui32 iterations = xyz_len * xyz_len * xyz_len;
        
        // Prepare chunks.

        hmem::Handle<hvox::ChunkBlockPager>        block_pager = hmem::make_handle<hvox::ChunkBlockPager>();
        hmem::Handle<hvox::ChunkInstanceDataPager> instance_data_pager = hmem::make_handle<hvox::ChunkInstanceDataPager>();

        hmem::PagedAllocator<hvox::Chunk, 4 * 4 * 4, 3> chunk_allocator;

        hmem::Handle<hvox::Chunk>* chunks = new hmem::Handle<hvox::Chunk>[iterations];

        for (ui32 x = 0; x < xyz_len; ++x) {
            for (ui32 y = 0; y < xyz_len; ++y) {
                for (ui32 z = 0; z < xyz_len; ++z) {
                    ui32 idx = x + y * xyz_len + z * xyz_len * xyz_len;
                    chunks[idx] = hmem::allocate_handle<hvox::Chunk>(chunk_allocator);
                    chunks[idx]->position = {x, y, z};
                    chunks[idx]->init(chunks[idx], block_pager, instance_data_pager);
                }
            }
        }

        // Prepare generator.

        const htest::performance_screen::VoxelGeneratorV2 generator{};

        // Do generation profiling.
        {
            auto start = std::chrono::high_resolution_clock::now();
            for (ui32 iteration = 0; iteration < iterations; ++iteration) {
                generator(chunks[iteration]);
            }
            auto duration = std::chrono::high_resolution_clock::now() - start;
            auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

            auto avg_duration_us = static_cast<f32>(duration_us) / static_cast<f32>(iterations);

            std::string msg = "Average per-chunk time: " + std::to_string(avg_duration_us) + "us";
            m_sprite_batcher.add_string(
                msg.c_str(),
                f32v4{ 40.0f, 60.0f, 1000.0f, 100.0f },
                f32v4{ 35.0f, 55.0f, 1010.0f, 110.0f },
                hg::f::StringSizing{ hg::f::StringSizingKind::SCALED, { f32v2{ 0.85f } } },
                colour4{ 0, 0, 0, 255 },
                "fonts/Orbitron-Regular.ttf",
                hg::f::TextAlign::TOP_LEFT,
                hg::f::WordWrap::NONE
            );
            m_sprite_batcher.end();
        }

        // Do naive meshing profiling.
        {
            auto start = std::chrono::high_resolution_clock::now();
            for (ui32 iteration = 0; iteration < iterations; ++iteration) {
                generator(chunks[iteration]);
            }
            auto duration = std::chrono::high_resolution_clock::now() - start;
            auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

            auto avg_duration_us = static_cast<f32>(duration_us) / static_cast<f32>(iterations);

            std::string msg = "Average per-chunk time: " + std::to_string(avg_duration_us) + "us";
            m_sprite_batcher.add_string(
                msg.c_str(),
                f32v4{ 40.0f, 120.0f, 1000.0f, 100.0f },
                f32v4{ 35.0f, 115.0f, 1010.0f, 110.0f },
                hg::f::StringSizing{ hg::f::StringSizingKind::SCALED, { f32v2{ 0.85f } } },
                colour4{ 0, 0, 0, 255 },
                "fonts/Orbitron-Regular.ttf",
                hg::f::TextAlign::TOP_LEFT,
                hg::f::WordWrap::NONE
            );
            m_sprite_batcher.end();
        }

        // Do greedy meshing profiling.
        {
            auto start = std::chrono::high_resolution_clock::now();
            for (ui32 iteration = 0; iteration < iterations; ++iteration) {
                generator(chunks[iteration]);
            }
            auto duration = std::chrono::high_resolution_clock::now() - start;
            auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

            auto avg_duration_us = static_cast<f32>(duration_us) / static_cast<f32>(iterations);

            std::string msg = "Average per-chunk time: " + std::to_string(avg_duration_us) + "us";
            m_sprite_batcher.add_string(
                msg.c_str(),
                f32v4{ 40.0f, 180.0f, 1000.0f, 100.0f },
                f32v4{ 35.0f, 175.0f, 1010.0f, 110.0f },
                hg::f::StringSizing{ hg::f::StringSizingKind::SCALED, { f32v2{ 0.85f } } },
                colour4{ 0, 0, 0, 255 },
                "fonts/Orbitron-Regular.ttf",
                hg::f::TextAlign::TOP_LEFT,
                hg::f::WordWrap::NONE
            );
            m_sprite_batcher.end();
        }


        // Force compiler to not optimise away intermediate results.
        ui32 rand_chunk_idx = static_cast<ui32>(std::floor(hemlock::global_unitary_rand<f32>() * static_cast<f32>(iterations)));
        std::cout << "Generation profiling complete." << std::endl;
        std::cout << "    - " << chunks[rand_chunk_idx]->blocks[0].id << std::endl;
    }

    virtual void update(hemlock::FrameTime) override {
        // Empty
    }

    virtual void draw(hemlock::FrameTime) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        happ::WindowDimensions dims = m_process->window()->dimensions();
        m_sprite_batcher.render(f32v2{ dims.width, dims.height });
    }

    virtual void init(const std::string& name, happ::ProcessBase* process) override {
        happ::ScreenBase::init(name, process);

        m_state = happ::ScreenState::RUNNING;

        handle_key_down = hemlock::Subscriber<hui::KeyboardButtonEvent>{
            [&](hemlock::Sender, hui::KeyboardButtonEvent ev) {
                if (m_state != happ::ScreenState::RUNNING) return;

                switch (ev.physical_key) {
                    case hui::PhysicalKey::H_G:
                        m_do_gen_profile.store(true);
                        return;
                    case hui::PhysicalKey::H_M:
                        m_do_naive_mesh_profile.store(true);
                        return;
                    case hui::PhysicalKey::H_R:
                        m_do_greedy_mesh_profile.store(true);
                        return;
                    case hui::PhysicalKey::H_N:
                        m_do_navmesh_profile.store(true);
                        return;
                    default:
                        break;
                }
            }
        };

        hui::InputDispatcher* dispatcher     = hui::InputDispatcher::instance();
        dispatcher->on_keyboard.button_down += &handle_key_down;

        m_shader_cache.init(
            &m_iom,
            hg::ShaderCache::Parser{
                [](const hio::fs::path& path, hio::IOManagerBase* iom) -> std::string {
                    std::string buffer;
                    if (!iom->read_file_to_string(path, buffer)) return "";

                    return buffer;
                } }
        );

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
        font->set_default_size(22);
        font->generate();

        m_sprite_batcher.init(&m_shader_cache, &m_font_cache);

        m_sprite_batcher.begin();
        m_sprite_batcher.add_string(
            "Generation Profiling",
            f32v4{ 30.0f, 30.0f, 1000.0f, 100.0f },
            f32v4{ 25.0f, 25.0f, 1010.0f, 110.0f },
            hg::f::StringSizing{ hg::f::StringSizingKind::SCALED, { f32v2{ 1.0f } } },
            colour4{ 0, 0, 0, 255 },
            "fonts/Orbitron-Regular.ttf",
            hg::f::TextAlign::TOP_LEFT,
            hg::f::WordWrap::NONE
        );
        m_sprite_batcher.add_string(
            "Naive Mesh Profiling",
            f32v4{ 30.0f, 90.0f, 1000.0f, 100.0f },
            f32v4{ 25.0f, 85.0f, 1010.0f, 110.0f },
            hg::f::StringSizing{ hg::f::StringSizingKind::SCALED, { f32v2{ 1.0f } } },
            colour4{ 0, 0, 0, 255 },
            "fonts/Orbitron-Regular.ttf",
            hg::f::TextAlign::TOP_LEFT,
            hg::f::WordWrap::NONE
        );
        m_sprite_batcher.add_string(
            "Greedy Mesh Profiling",
            f32v4{ 30.0f, 150.0f, 1000.0f, 100.0f },
            f32v4{ 25.0f, 145.0f, 1010.0f, 110.0f },
            hg::f::StringSizing{ hg::f::StringSizingKind::SCALED, { f32v2{ 1.0f } } },
            colour4{ 0, 0, 0, 255 },
            "fonts/Orbitron-Regular.ttf",
            hg::f::TextAlign::TOP_LEFT,
            hg::f::WordWrap::NONE
        );
        m_sprite_batcher.add_string(
            "Navigation Mesh Profiling",
            f32v4{ 30.0f, 210.0f, 1000.0f, 100.0f },
            f32v4{ 25.0f, 205.0f, 1010.0f, 110.0f },
            hg::f::StringSizing{ hg::f::StringSizingKind::SCALED, { f32v2{ 1.0f } } },
            colour4{ 0, 0, 0, 255 },
            "fonts/Orbitron-Regular.ttf",
            hg::f::TextAlign::TOP_LEFT,
            hg::f::WordWrap::NONE
        );
        m_sprite_batcher.end();
    }
protected:
    hemlock::Subscriber<hui::KeyboardButtonEvent> handle_key_down;
    MyIOManager                      m_iom;
    hg::ShaderCache                  m_shader_cache;
    hg::f::FontCache                 m_font_cache;
    hg::s::SpriteBatcher             m_sprite_batcher;
};

#endif  // __hemlock_tests_test_performance_screen_hpp

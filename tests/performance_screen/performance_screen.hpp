#ifndef __hemlock_tests_test_performance_screen_hpp
#define __hemlock_tests_test_performance_screen_hpp

#include <FastNoise/FastNoise.h>

#include "graphics/font/font.h"
#include "graphics/glsl_program.h"
#include "graphics/sprite/batcher.h"
#include "memory/handle.hpp"
#include "rand.h"
#include "ui/input/dispatcher.h"
#include "voxel/ai/navmesh/navmesh_task.hpp"
#include "voxel/ai/navmesh/strategy/naive/strategy.hpp"
#include "voxel/chunk/state.hpp"
#include "voxel/generation/generator_task.hpp"
#include "voxel/graphics/mesh/greedy_strategy.hpp"
#include "voxel/graphics/mesh/instance_manager.h"
#include "voxel/graphics/mesh/naive_strategy.hpp"

#include "tests/iomanager.hpp"

#include "tests/performance_screen/terrain.hpp"

// Note(Matthew): Some thoughts about chunk prep timings. If we think of modern trains,
//                for which the upper speed is roughly 600Km/h, we can calculate that
//                for a 20 chunk view distance, in a game using the half-metre per voxel
//                metric (so 320m view distance), we pessimistically need to prepare
//                ~7000 chunks per second. With current (Apr 2023) timings, we have a
//                ~1.4ms per chunk cost, which allows for ~720 chunks a second per
//                thread on GCC and ~0.9ms for ~1100 chunks a second per thread on
//                Clang. We can expect to target future hardware with 12 cores and
//                upward, but we'd in those circumstances need to provide 10 (GCC) or
//                7 (Clang) threads to chunk preparation alone, ignoring any systems to
//                maintain the local environment experience and any future additional
//                chunk prep costs.
//                  Suffice to say, we would like to bring these timings down closer to
//                  0.7ms (=> 5 threads needed) to give any tricks we would need to
//                  employ more slack to work.

class TestPerformanceScreen : public happ::ScreenBase {
public:
    TestPerformanceScreen() : happ::ScreenBase(), m_do_profile(false) { /* Empty. */
    }

    virtual ~TestPerformanceScreen() {
        auto font = m_font_cache.fetch("fonts/Orbitron-Regular.ttf");
        font->dispose();
    };

    virtual void start(hemlock::FrameTime time) override {
        happ::ScreenBase::start(time);
    }

    virtual void update(hemlock::FrameTime) override {
        if (m_do_profile) {
#if defined(DEBUG)
            const ui32 xyz_len = 4;
#else
            const ui32 xyz_len = 18;
#endif
            const ui32 iterations = xyz_len * xyz_len * xyz_len;

            // Prepare chunks.

            hmem::Handle<hvox::ChunkBlockPager> block_pager
                = hmem::make_handle<hvox::ChunkBlockPager>();
            hmem::Handle<hvox::ChunkInstanceDataPager> instance_pager
                = hmem::make_handle<hvox::ChunkInstanceDataPager>();
            hmem::Handle<hvox::ai::ChunkNavmeshPager> navmesh_pager
                = hmem::make_handle<hvox::ai::ChunkNavmeshPager>();

            hmem::PagedAllocator<hvox::Chunk, 4 * 4 * 4, 3> chunk_allocator;

            hmem::Handle<hvox::Chunk>* chunks
                = new hmem::Handle<hvox::Chunk>[iterations];

            auto chunk_idx = [](ui32 x, ui32 y, ui32 z) {
                return x + y * xyz_len + z * xyz_len * xyz_len;
            };

            for (ui32 x = 0; x < xyz_len; ++x) {
                for (ui32 y = 0; y < xyz_len; ++y) {
                    for (ui32 z = 0; z < xyz_len; ++z) {
                        ui32 idx = chunk_idx(x, y, z);
                        chunks[idx]
                            = hmem::allocate_handle<hvox::Chunk>(chunk_allocator);
                        chunks[idx]->position = {
                            {x, y, z}
                        };
                        chunks[idx]->init(
                            chunks[idx], block_pager, instance_pager, navmesh_pager
                        );
                    }
                }
            }

            for (ui32 x = 0; x < xyz_len; ++x) {
                for (ui32 y = 0; y < xyz_len; ++y) {
                    for (ui32 z = 0; z < xyz_len; ++z) {
                        ui32 idx = chunk_idx(x, y, z);

                        if (x != 0)
                            chunks[idx]->neighbours.one.left
                                = chunks[chunk_idx(x - 1, y, z)];
                        if (x != xyz_len - 1)
                            chunks[idx]->neighbours.one.right
                                = chunks[chunk_idx(x + 1, y, z)];
                        if (y != 0)
                            chunks[idx]->neighbours.one.bottom
                                = chunks[chunk_idx(x, y - 1, z)];
                        if (y != xyz_len - 1)
                            chunks[idx]->neighbours.one.top
                                = chunks[chunk_idx(x, y + 1, z)];
                        if (z != 0)
                            chunks[idx]->neighbours.one.back
                                = chunks[chunk_idx(x, y, z - 1)];
                        if (z != xyz_len - 1)
                            chunks[idx]->neighbours.one.front
                                = chunks[chunk_idx(x, y, z + 1)];
                    }
                }
            }

            // const htest::performance_screen::VoxelGenerator generator{};
            const htest::performance_screen::VoxelGeneratorV2 generator{};

            // Do generation profiling.
            {
                auto start = std::chrono::high_resolution_clock::now();
                for (ui32 iteration = 0; iteration < iterations; ++iteration) {
                    generator(chunks[iteration]);
                }
                auto duration = std::chrono::high_resolution_clock::now() - start;
                auto duration_us
                    = std::chrono::duration_cast<std::chrono::microseconds>(duration)
                          .count();

                auto avg_duration_us
                    = static_cast<f32>(duration_us) / static_cast<f32>(iterations);

                std::string msg = "Average per-chunk time: "
                                  + std::to_string(avg_duration_us) + "us";
                m_sprite_batcher.add_string(
                    msg.c_str(),
                    f32v4{ 40.0f, 60.0f, 1000.0f, 100.0f },
                    f32v4{ 35.0f, 55.0f, 1010.0f, 110.0f },
                    hg::f::StringSizing{ hg::f::StringSizingKind::SCALED,
                                         { f32v2{ 0.85f } } },
                    colour4{ 0, 0, 0, 255 },
                    "fonts/Orbitron-Regular.ttf",
                    hg::f::TextAlign::TOP_LEFT,
                    hg::f::WordWrap::NONE
                );
                m_sprite_batcher.end();
            }

            // Force compiler to not optimise away intermediate results.
            {
                ui32 rand_chunk_idx = static_cast<ui32>(std::floor(
                    hemlock::global_unitary_rand<f32>() * static_cast<f32>(iterations)
                ));
                ui32 rand_block_idx = static_cast<ui32>(std::floor(
                    hemlock::global_unitary_rand<f32>() * static_cast<f32>(CHUNK_VOLUME)
                ));

                std::shared_lock<std::shared_mutex> lock;
                auto blocks = chunks[rand_chunk_idx]->blocks.get(lock);

                std::cout << "    - " << blocks[rand_block_idx].id << std::endl;
            }

            const hvox::NaiveMeshStrategy<htest::performance_screen::BlockComparator>
                naive_mesh;

            // Do naive meshing profiling.
            {
                auto start = std::chrono::high_resolution_clock::now();
                for (ui32 iteration = 0; iteration < iterations; ++iteration) {
                    naive_mesh({}, chunks[iteration]);
                }
                auto duration = std::chrono::high_resolution_clock::now() - start;
                auto duration_us
                    = std::chrono::duration_cast<std::chrono::microseconds>(duration)
                          .count();

                auto avg_duration_us
                    = static_cast<f32>(duration_us) / static_cast<f32>(iterations);

                std::string msg = "Average per-chunk time: "
                                  + std::to_string(avg_duration_us) + "us";
                m_sprite_batcher.add_string(
                    msg.c_str(),
                    f32v4{ 40.0f, 120.0f, 1000.0f, 100.0f },
                    f32v4{ 35.0f, 115.0f, 1010.0f, 110.0f },
                    hg::f::StringSizing{ hg::f::StringSizingKind::SCALED,
                                         { f32v2{ 0.85f } } },
                    colour4{ 0, 0, 0, 255 },
                    "fonts/Orbitron-Regular.ttf",
                    hg::f::TextAlign::TOP_LEFT,
                    hg::f::WordWrap::NONE
                );
                m_sprite_batcher.end();
            }

            // Force compiler to not optimise away intermediate results.
            {
                ui32 rand_chunk_idx = static_cast<ui32>(std::floor(
                    hemlock::global_unitary_rand<f32>() * static_cast<f32>(iterations)
                ));

                std::shared_lock<std::shared_mutex> lock;
                auto instance = chunks[rand_chunk_idx]->instance.get(lock);

                ui32 rand_instance_idx = static_cast<ui32>(std::floor(
                    hemlock::global_unitary_rand<f32>()
                    * static_cast<f32>(instance.count)
                ));

                std::cout << "    - " << instance.data[rand_instance_idx].translation.x
                          << std::endl;
            }

            const hvox::GreedyMeshStrategy<htest::performance_screen::BlockComparator>
                greedy_mesh;

            // Do greedy meshing profiling.
            {
                auto start = std::chrono::high_resolution_clock::now();
                for (ui32 iteration = 0; iteration < iterations; ++iteration) {
                    greedy_mesh({}, chunks[iteration]);

                    chunks[iteration]->meshing.store(
                        hvox::ChunkState::COMPLETE, std::memory_order_release
                    );
                }
                auto duration = std::chrono::high_resolution_clock::now() - start;
                auto duration_us
                    = std::chrono::duration_cast<std::chrono::microseconds>(duration)
                          .count();

                auto avg_duration_us
                    = static_cast<f32>(duration_us) / static_cast<f32>(iterations);

                std::string msg = "Average per-chunk time: "
                                  + std::to_string(avg_duration_us) + "us";
                m_sprite_batcher.add_string(
                    msg.c_str(),
                    f32v4{ 40.0f, 180.0f, 1000.0f, 100.0f },
                    f32v4{ 35.0f, 175.0f, 1010.0f, 110.0f },
                    hg::f::StringSizing{ hg::f::StringSizingKind::SCALED,
                                         { f32v2{ 0.85f } } },
                    colour4{ 0, 0, 0, 255 },
                    "fonts/Orbitron-Regular.ttf",
                    hg::f::TextAlign::TOP_LEFT,
                    hg::f::WordWrap::NONE
                );
                m_sprite_batcher.end();
            }

            // Force compiler to not optimise away intermediate results.
            {
                ui32 rand_chunk_idx = static_cast<ui32>(std::floor(
                    hemlock::global_unitary_rand<f32>() * static_cast<f32>(iterations)
                ));

                std::shared_lock<std::shared_mutex> lock;
                auto instance = chunks[rand_chunk_idx]->instance.get(lock);

                ui32 rand_instance_idx = static_cast<ui32>(std::floor(
                    hemlock::global_unitary_rand<f32>()
                    * static_cast<f32>(instance.count)
                ));

                std::cout << "    - " << instance.data[rand_instance_idx].translation.x
                          << std::endl;
            }

            const hvox::ai::NaiveNavmeshStrategy<
                htest::performance_screen::BlockSolidCheck>
                naive_navmesh;

            // Do naive navmesh profiling.
            {
                auto bulk_start = std::chrono::high_resolution_clock::now();
                for (ui32 iteration = 0; iteration < iterations; ++iteration) {
                    naive_navmesh.do_bulk({}, chunks[iteration]);

                    chunks[iteration]->bulk_navmeshing.store(
                        hvox::ChunkState::COMPLETE, std::memory_order_release
                    );
                }
                auto bulk_duration
                    = std::chrono::high_resolution_clock::now() - bulk_start;
                auto bulk_duration_us
                    = std::chrono::duration_cast<std::chrono::microseconds>(
                          bulk_duration
                    )
                          .count();

                auto bulk_avg_duration_us
                    = static_cast<f32>(bulk_duration_us) / static_cast<f32>(iterations);

                auto stitch_start = std::chrono::high_resolution_clock::now();
                for (ui32 iteration = 0; iteration < iterations; ++iteration) {
                    naive_navmesh.do_stitch({}, chunks[iteration]);

                    chunks[iteration]->navmeshing.store(
                        hvox::ChunkState::COMPLETE, std::memory_order_release
                    );
                }
                auto stitch_duration
                    = std::chrono::high_resolution_clock::now() - stitch_start;
                auto stitch_duration_us
                    = std::chrono::duration_cast<std::chrono::microseconds>(
                          stitch_duration
                    )
                          .count();

                auto stitch_avg_duration_us = static_cast<f32>(stitch_duration_us)
                                              / static_cast<f32>(iterations);

                std::string msg = "Average per-chunk time: bulk "
                                  + std::to_string(bulk_avg_duration_us)
                                  + "us // stitch "
                                  + std::to_string(stitch_avg_duration_us) + "us";
                m_sprite_batcher.add_string(
                    msg.c_str(),
                    f32v4{ 40.0f, 240.0f, 1000.0f, 100.0f },
                    f32v4{ 35.0f, 235.0f, 1010.0f, 110.0f },
                    hg::f::StringSizing{ hg::f::StringSizingKind::SCALED,
                                         { f32v2{ 0.85f } } },
                    colour4{ 0, 0, 0, 255 },
                    "fonts/Orbitron-Regular.ttf",
                    hg::f::TextAlign::TOP_LEFT,
                    hg::f::WordWrap::NONE
                );
                m_sprite_batcher.end();
            }

            // Force compiler to not optimise away intermediate results.
            {
                ui32 rand_chunk_idx = static_cast<ui32>(std::floor(
                    hemlock::global_unitary_rand<f32>() * static_cast<f32>(iterations)
                ));

                std::shared_lock<std::shared_mutex> lock;
                auto navmesh = chunks[rand_chunk_idx]->navmesh.get(lock);

                std::cout << "    - " << boost::num_vertices(navmesh->graph)
                          << std::endl;
            }

            {
                size_t allocated_bytes = block_pager->allocated_bytes()
                                         + instance_pager->allocated_bytes()
                                         + chunk_allocator.allocated_bytes();
                size_t allocated_MB = allocated_bytes / 1000000;

                std::string msg
                    = "Memory consumption: " + std::to_string(allocated_MB) + "MB";
                m_sprite_batcher.add_string(
                    msg.c_str(),
                    f32v4{ 40.0f, 300.0f, 1000.0f, 100.0f },
                    f32v4{ 35.0f, 295.0f, 1010.0f, 110.0f },
                    hg::f::StringSizing{ hg::f::StringSizingKind::SCALED,
                                         { f32v2{ 0.85f } } },
                    colour4{ 0, 0, 0, 255 },
                    "fonts/Orbitron-Regular.ttf",
                    hg::f::TextAlign::TOP_LEFT,
                    hg::f::WordWrap::NONE
                );
                m_sprite_batcher.end();
            }

            std::cout << "Generation profiling complete." << std::endl;

            delete[] chunks;
            block_pager->dispose();
            instance_pager->dispose();

            m_do_profile.store(false);
        }
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
                    case hui::PhysicalKey::H_SPACE:
                        m_do_profile.store(true);
                        return;
                    default:
                        break;
                }
            }
        };

        hui::InputDispatcher* dispatcher    = hui::InputDispatcher::instance();
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
    MyIOManager                                   m_iom;
    hg::ShaderCache                               m_shader_cache;
    hg::f::FontCache                              m_font_cache;
    hg::s::SpriteBatcher                          m_sprite_batcher;

    std::atomic<bool> m_do_profile;
};

#endif  // __hemlock_tests_test_performance_screen_hpp

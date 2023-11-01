#ifndef __hemlock_tests_navmesh_screen_terrain_generation_hpp
#define __hemlock_tests_navmesh_screen_terrain_generation_hpp

namespace hemlock {
    namespace test {
        namespace navmesh_screen {
            struct VoxelGenerator {
                void operator()(hmem::Handle<hvox::Chunk> chunk) const {
                    auto simplex_1      = FastNoise::New<FastNoise::Simplex>();
                    auto fractal_1      = FastNoise::New<FastNoise::FractalFBm>();
                    auto domain_scale_1 = FastNoise::New<FastNoise::DomainScale>();
                    auto position_output_1
                        = FastNoise::New<FastNoise::PositionOutput>();
                    auto add_1 = FastNoise::New<FastNoise::Add>();
                    auto domain_warp_grad_1
                        = FastNoise::New<FastNoise::DomainWarpGradient>();
                    auto domain_warp_fract_prog_1
                        = FastNoise::New<FastNoise::DomainWarpFractalProgressive>();

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

                    {
                        std::unique_lock<std::shared_mutex> lock;
                        auto blocks = chunk->blocks.get(lock);

                        ui64 noise_idx = 0;
                        for (ui8 z = 0; z < CHUNK_LENGTH; ++z) {
                            for (ui8 y = 0; y < CHUNK_LENGTH; ++y) {
                                for (ui8 x = 0; x < CHUNK_LENGTH; ++x) {
                                    blocks[hvox::block_index(
                                        { x, CHUNK_LENGTH - y - 1, z }
                                    )] = data[noise_idx++] > 0 ? hvox::Block{ 1 } :
                                                                 hvox::Block{ 0 };
                                }
                            }
                        }
                    }

                    delete[] data;
                }
            };

            void load_chunks(hmem::Handle<hvox::ChunkGrid> chunk_grid) {
                for (auto x = -VIEW_DIST; x <= VIEW_DIST; ++x) {
                    for (auto z = -VIEW_DIST; z <= VIEW_DIST; ++z) {
                        for (auto y = -2; y < 6; ++y) {
                            chunk_grid->preload_chunk_at({
                                {x, y, z}
                            });
                        }
                    }
                }
                for (auto x = -VIEW_DIST; x <= VIEW_DIST; ++x) {
                    for (auto z = -VIEW_DIST; z <= VIEW_DIST; ++z) {
                        for (auto y = -2; y < 6; ++y) {
                            chunk_grid->load_chunk_at({
                                {x, y, z}
                            });
                        }
                    }
                }
            }

            void unload_x_chunks(
                hmem::Handle<hvox::ChunkGrid>               chunk_grid,
                std::vector<hmem::WeakHandle<hvox::Chunk>>& unloading_chunks,
                const f32v3&                                current_pos,
                const f32v3&                                last_pos
            ) {
                i32 x_step
                    = static_cast<i32>(current_pos.x) - static_cast<i32>(last_pos.x);
                if (x_step != 0) {
                    for (auto z = static_cast<i32>(current_pos.z) - VIEW_DIST;
                         z <= static_cast<i32>(current_pos.z) + VIEW_DIST;
                         ++z)
                    {
                        for (auto y = -2; y < 6; ++y) {
                            unloading_chunks.emplace_back(
                                hmem::WeakHandle<hvox::Chunk>{}
                            );
                            auto& handle = unloading_chunks.back();
                            chunk_grid->unload_chunk_at(
                                {
                                    {x_step < 0 ? static_cast<i32>(current_pos.x)
+ VIEW_DIST + 1 :
static_cast<i32>(current_pos.x)
- VIEW_DIST - 1,
                                     y, z}
                            },
                                &handle
                            );
                        }
                    }
                    for (auto z = static_cast<i32>(current_pos.z) - VIEW_DIST;
                         z <= static_cast<i32>(current_pos.z) + VIEW_DIST;
                         ++z)
                    {
                        for (auto y = -2; y < 6; ++y) {
                            chunk_grid->preload_chunk_at({
                                {x_step < 0 ?
static_cast<i32>(current_pos.x) - VIEW_DIST :
static_cast<i32>(current_pos.x) + VIEW_DIST,
                                 y, z}
                            });
                        }
                    }
                    for (auto z = static_cast<i32>(current_pos.z) - VIEW_DIST;
                         z <= static_cast<i32>(current_pos.z) + VIEW_DIST;
                         ++z)
                    {
                        for (auto y = -2; y < 6; ++y) {
                            chunk_grid->load_chunk_at({
                                {x_step < 0 ?
static_cast<i32>(current_pos.x) - VIEW_DIST :
static_cast<i32>(current_pos.x) + VIEW_DIST,
                                 y, z}
                            });
                        }
                    }
                }
            }

            void unload_z_chunks(
                hmem::Handle<hvox::ChunkGrid>               chunk_grid,
                std::vector<hmem::WeakHandle<hvox::Chunk>>& unloading_chunks,
                const f32v3&                                current_pos,
                const f32v3&                                last_pos
            ) {
                i32 z_step
                    = static_cast<i32>(current_pos.z) - static_cast<i32>(last_pos.z);
                if (z_step != 0) {
                    for (auto x = static_cast<i32>(current_pos.x) - VIEW_DIST;
                         x <= static_cast<i32>(current_pos.x) + VIEW_DIST;
                         ++x)
                    {
                        for (auto y = -2; y < 6; ++y) {
                            unloading_chunks.emplace_back(
                                hmem::WeakHandle<hvox::Chunk>{}
                            );
                            auto& handle = unloading_chunks.back();
                            chunk_grid->unload_chunk_at(
                                {
                                    {x,
                                     y, z_step < 0 ? static_cast<i32>(current_pos.z)
 + VIEW_DIST + 1 :
 static_cast<i32>(current_pos.z)
 - VIEW_DIST - 1}
                            },
                                &handle
                            );
                        }
                    }
                    for (auto x = static_cast<i32>(current_pos.x) - VIEW_DIST;
                         x <= static_cast<i32>(current_pos.x) + VIEW_DIST;
                         ++x)
                    {
                        for (auto y = -2; y < 6; ++y) {
                            chunk_grid->preload_chunk_at({
                                {x,
                                 y, z_step < 0 ?
 static_cast<i32>(current_pos.z) - VIEW_DIST :
 static_cast<i32>(current_pos.z) + VIEW_DIST}
                            });
                        }
                    }
                    for (auto x = static_cast<i32>(current_pos.x) - VIEW_DIST;
                         x <= static_cast<i32>(current_pos.x) + VIEW_DIST;
                         ++x)
                    {
                        for (auto y = -2; y < 6; ++y) {
                            chunk_grid->load_chunk_at({
                                {x,
                                 y, z_step < 0 ?
 static_cast<i32>(current_pos.z) - VIEW_DIST :
 static_cast<i32>(current_pos.z) + VIEW_DIST}
                            });
                        }
                    }
                }
            }
        }  // namespace navmesh_screen
    }      // namespace test
}  // namespace hemlock
namespace htest = hemlock::test;

#endif  // __hemlock_tests_navmesh_screen_terrain_generation_hpp
#ifndef __hemlock_tests_performance_screen_terrain_generation_hpp
#define __hemlock_tests_performance_screen_terrain_generation_hpp

namespace hemlock {
    namespace test {
        namespace performance_screen {
            struct VoxelGenerator {
                void operator()(hmem::Handle<hvox::Chunk> chunk) const {
                    auto simplex_1
                        = FastNoise::New<FastNoise::Simplex>(FastSIMD::Level_AVX512);
                    auto fractal_1
                        = FastNoise::New<FastNoise::FractalFBm>(FastSIMD::Level_AVX512);
                    auto domain_scale_1
                        = FastNoise::New<FastNoise::DomainScale>(FastSIMD::Level_AVX512
                        );
                    auto position_output_1 = FastNoise::New<FastNoise::PositionOutput>(
                        FastSIMD::Level_AVX512
                    );
                    auto add_1 = FastNoise::New<FastNoise::Add>(FastSIMD::Level_AVX512);
                    auto domain_warp_grad_1
                        = FastNoise::New<FastNoise::DomainWarpGradient>(
                            FastSIMD::Level_AVX512
                        );
                    auto domain_warp_fract_prog_1
                        = FastNoise::New<FastNoise::DomainWarpFractalProgressive>(
                            FastSIMD::Level_AVX512
                        );

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

                    // With new.
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

            struct VoxelGeneratorV2 {
                VoxelGeneratorV2() { m_data = new f32[CHUNK_VOLUME]; }

                ~VoxelGeneratorV2() { delete[] m_data; }

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

                    domain_warp_fract_prog_1->GenUniformGrid3D(
                        m_data,
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
                                    )] = m_data[noise_idx++] > 0 ? hvox::Block{ 1 } :
                                                                   hvox::Block{ 0 };
                                }
                            }
                        }
                    }
                }
            protected:
                f32* m_data = nullptr;
            };
        }  // namespace performance_screen
    }      // namespace test
}  // namespace hemlock
namespace htest = hemlock::test;

#endif  // __hemlock_tests_performance_screen_terrain_generation_hpp
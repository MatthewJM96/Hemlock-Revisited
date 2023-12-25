#ifndef __hemlock_tests_performance_screen_terrain_meshing_hpp
#define __hemlock_tests_performance_screen_terrain_meshing_hpp

namespace hemlock {
    namespace test {
        namespace performance_screen {
            struct VoxelComparator {
                bool
                operator()(hvox::Voxel source, hvox::Voxel target, hvox::VoxelChunkPosition, hvox::Chunk*)
                    const {
                    return (source == target) && (source != hvox::NULL_VOXEL);
                }
            };

            struct VoxelSolidCheck {
                bool operator()(hvox::Voxel voxel) const {
                    return voxel != hvox::NULL_VOXEL;
                }
            };
        }  // namespace performance_screen
    }      // namespace test
}  // namespace hemlock
namespace htest = hemlock::test;

#endif  // __hemlock_tests_performance_screen_terrain_meshing_hpp

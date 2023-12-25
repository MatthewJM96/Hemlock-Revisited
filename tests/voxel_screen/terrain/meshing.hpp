#ifndef __hemlock_tests_voxel_screen_terrain_meshing_hpp
#define __hemlock_tests_voxel_screen_terrain_meshing_hpp

namespace hemlock {
    namespace test {
        namespace voxel_screen {
            struct TVS_VoxelComparator {
                bool
                operator()(hvox::Voxel source, hvox::Voxel target, hvox::VoxelChunkPosition, hvox::Chunk*)
                    const {
                    return (source == target) && (source != hvox::NULL_VOXEL);
                }
            };
        }  // namespace voxel_screen
    }      // namespace test
}  // namespace hemlock
namespace htest = hemlock::test;

#endif  // __hemlock_tests_voxel_screen_terrain_meshing_hpp

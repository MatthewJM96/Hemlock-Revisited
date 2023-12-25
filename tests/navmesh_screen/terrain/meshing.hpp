#ifndef __hemlock_tests_navmesh_screen_terrain_meshing_hpp
#define __hemlock_tests_navmesh_screen_terrain_meshing_hpp

namespace hemlock {
    namespace test {
        namespace navmesh_screen {
            struct VoxelComparator {
                bool
                operator()(const hvox::Voxel* source, const hvox::Voxel* target, hvox::VoxelChunkPosition, hvox::Chunk*)
                    const {
                    return (source->id == target->id) && (source->id != 0);
                }
            };

            struct VoxelSolidCheck {
                bool operator()(const hvox::Voxel* voxel) const {
                    return voxel->id != 0;
                }
            };
        }  // namespace navmesh_screen
    }      // namespace test
}  // namespace hemlock
namespace htest = hemlock::test;

#endif  // __hemlock_tests_navmesh_screen_terrain_meshing_hpp

#ifndef __hemlock_tests_voxel_screen_terrain_meshing_hpp
#define __hemlock_tests_voxel_screen_terrain_meshing_hpp

namespace hemlock {
    namespace test {
        namespace voxel_screen {
            struct TVS_BlockComparator {
                bool operator()(const hvox::Block* source, const hvox::Block* target, hvox::BlockChunkPosition, hvox::Chunk*) const {
                    return (source->id == target->id) && (source->id != 0);
                }
            };
        }
    }
}
namespace htest = hemlock::test;

#endif // __hemlock_tests_voxel_screen_terrain_meshing_hpp

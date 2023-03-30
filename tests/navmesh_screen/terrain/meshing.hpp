#ifndef __hemlock_tests_navmesh_screen_terrain_meshing_hpp
#define __hemlock_tests_navmesh_screen_terrain_meshing_hpp

namespace hemlock {
    namespace test {
        namespace navmesh_screen {
            struct BlockComparator {
                bool
                operator()(const hvox::Block* source, const hvox::Block* target, hvox::BlockChunkPosition, hvox::Chunk*)
                    const {
                    return (source->id == target->id) && (source->id != 0);
                }
            };

            struct BlockSolidCheck {
                bool operator()(const hvox::Block* block) const {
                    return block->id != 0;
                }
            };
        }  // namespace navmesh_screen
    }      // namespace test
}  // namespace hemlock
namespace htest = hemlock::test;

#endif  // __hemlock_tests_navmesh_screen_terrain_meshing_hpp

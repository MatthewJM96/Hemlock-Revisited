#ifndef __hemlock_voxel_chunk_state_hpp
#define __hemlock_voxel_chunk_state_hpp

namespace hemlock {
    namespace voxel {
        enum class ChunkState : ui8 {
            NONE     = 0,
            PENDING  = 1,
            ACTIVE   = 2,
            COMPLETE = 3
        };

        enum class ChunkAliveState : ui8 {
            ALIVE,
            DEAD
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_state_hpp

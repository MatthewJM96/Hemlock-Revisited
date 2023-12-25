#ifndef __hemlock_voxel_chunk_event_voxel_change_hpp
#define __hemlock_voxel_chunk_event_voxel_change_hpp

#include "voxel/coordinate_system.h"
#include "voxel/state.h"

namespace hemlock {
    namespace voxel {
        struct VoxelChangeEvent {
            Voxel              old_voxel;
            Voxel              new_voxel;
            VoxelChunkPosition voxel_position;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_event_voxel_change_hpp

#ifndef __hemlock_voxel_chunk_event_bulk_voxel_change_hpp
#define __hemlock_voxel_chunk_event_bulk_voxel_change_hpp

#include "voxel/coordinate_system.h"
#include "voxel/state.hpp"

namespace hemlock {
    namespace voxel {
        struct BulkVoxelChangeEvent {
            Voxel*             new_voxels;
            bool               single_voxel;
            VoxelChunkPosition start_position;
            VoxelChunkPosition end_position;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_event_bulk_voxel_change_hpp

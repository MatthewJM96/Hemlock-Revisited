#ifndef __hemlock_voxel_chunk_events_block_change_hpp
#define __hemlock_voxel_chunk_events_block_change_hpp

#include "voxel/block.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct BlockChangeEvent {
            Block              old_block;
            Block              new_block;
            BlockChunkPosition block_position;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_events_block_change_hpp

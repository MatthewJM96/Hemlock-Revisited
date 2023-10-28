#ifndef __hemlock_voxel_chunk_events_bulk_block_change_hpp
#define __hemlock_voxel_chunk_events_bulk_block_change_hpp

#include "voxel/block.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct BulkBlockChangeEvent {
            Block*             new_blocks;
            bool               single_block;
            BlockChunkPosition start_position;
            BlockChunkPosition end_position;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_events_bulk_block_change_hpp

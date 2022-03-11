#ifndef __hemlock_voxel_chunk_events_hpp
#define __hemlock_voxel_chunk_events_hpp

#include "voxel/block.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        struct BlockChangeEvent {
            hmem::Handle<Chunk> chunk;
            Block               old_block;
            Block               new_block;
            BlockChunkPosition  block_position;
        };

        struct BulkBlockChangeEvent {
            hmem::Handle<Chunk> chunk;
            Block*              new_blocks;
            bool                single_block;
            BlockChunkPosition  start_position;
            BlockChunkPosition  end_position;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_events_hpp

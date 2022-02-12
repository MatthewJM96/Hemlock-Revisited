#ifndef __hemlock_voxel_state_hpp
#define __hemlock_voxel_state_hpp

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 32
#endif

#include "graphics/mesh.h"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Block;
        struct Chunk;

        enum class ChunkState {
            NONE,
            GENERATED,
            MESHED
        };

        enum class BlockChange {
            PLACE,
            DESTROY
        };

        struct BlockChangeEvent {
            Chunk*              chunk;
            BlockChange         change;
            BlockChunkPosition  block_position;
            ChunkGridPosition   chunk_position;
        };

        struct BulkBlockChangeEvent {
            Chunk*              chunk;
            const Block*        blocks;
            ui32                block_count;
            BlockChunkPosition  start_position;
            BlockChunkPosition  end_position;
            ChunkGridPosition   chunk_position;
        };

        /**
         * @brief 
         */
        struct Chunk {
            struct {
                Chunk *left, *right, *top, *bottom, *front, *back;
            } neighbours;

            Block* blocks;

            ChunkGridPosition position;

            ChunkState state;

            hg::MeshHandles mesh_handles;

            // TODO(Matthew): Store these here? Somehow feels dodgy.
            Event<BlockChangeEvent>     on_block_change;
            Event<BulkBlockChangeEvent> on_bulk_block_change;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_state_hpp

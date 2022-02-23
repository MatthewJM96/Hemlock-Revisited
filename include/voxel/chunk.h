#ifndef __hemlock_voxel_chunk_h
#define __hemlock_voxel_chunk_h

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 32
#endif

#include "timing.h"
#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/chunk/load_task.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;
        class ChunkGrid;

        // TODO(Matthew): We shoud instance block and upload scale/translation transformations only.
        //                  Initially just translation, scale comes after we optimise.
        enum class ChunkState : ui8 {
            NONE            = 0,
            PRELOADED       = 1,
            GENERATED       = 2,
            MESHED          = 3,
            MESH_UPLOADED   = 4
        };

        struct BlockChangeEvent {
            Chunk*              chunk;
            Block               old_block;
            Block               new_block;
            BlockChunkPosition  block_position;
        };

        struct BulkBlockChangeEvent {
            Chunk*              chunk;
            Block*              new_blocks;
            bool                single_block;
            BlockChunkPosition  start_position;
            BlockChunkPosition  end_position;
        };

        union Neighbours {
            struct {
                Chunk *left, *right, *top, *bottom, *front, *back;
            };
            Chunk* neighbours[8];
        };
        const Neighbours NULL_NEIGHBOURS = Neighbours{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

        struct ChunkInstanceData {
            f32v3 translation, scaling;
        };

        /**
         * @brief 
         */
        struct Chunk {
            Chunk();

            void init();
            void init(Block* _blocks);
            void dispose();

            void update(TimeData);

            ChunkGridPosition position;
            Neighbours        neighbours;
            Block*            blocks;

            struct {
                ChunkInstanceData*  data;
                ui32                count;
            } instance;

            std::atomic<ChunkState>        state;
            std::atomic<ChunkLoadTaskKind> pending_task;
            std::atomic<bool>               gen_task_active, mesh_task_active;

            // TODO(Matthew): Store these here? Somehow feels dodgy.
            CancellableEvent<BlockChangeEvent>     on_block_change;
            CancellableEvent<BulkBlockChangeEvent> on_bulk_block_change;
        protected:
            bool m_owns_blocks;
        };

        /**
         * @brief Set the block at the given position in the chunk
         * passed in.
         * 
         * @param chunk The chunk in which to set the block.
         * @param block_position The position at which to set the block.
         * @param block The block to set.
         *
         * @return True if the block was set, false otherwise.
         */
        bool set_block( Chunk* chunk,
            BlockChunkPosition block_position,
                         Block block );
        /**
         * @brief Set all points in a rectangular cuboid of the
         * passed in chunk to a specific block.
         * 
         * @param chunk The chunk in which to set the block.
         * @param block_position The position marking the start of
         * the rectangular cuboid to set blocks in.
         * @param block_position The position marking the end of
         * the rectangular cuboid to set blocks in.
         * @param block The block to set.
         *
         * @return True if the blocks were set, false otherwise.
         */
        bool set_blocks( Chunk* chunk,
             BlockChunkPosition start_block_position,
             BlockChunkPosition end_block_position,
                          Block block );
        /**
         * @brief Set all points in a rectangular cuboid of the
         * passed in chunk to each block in a buffer. Note, the
         * buffer is assumed to go in x, then y, then z starting
         * from the near bottom left of the chunk.
         * 
         * @param chunk The chunk in which to set the block.
         * @param block_position The position at which to set the block.
         * @param block The blocks to set.
         *
         * @return True if the blocks were set, false otherwise.
         */
        bool set_blocks( Chunk* chunk,
             BlockChunkPosition start_block_position,
             BlockChunkPosition end_block_position,
                         Block* blocks );
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_h

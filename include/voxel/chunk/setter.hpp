#ifndef __hemlock_voxel_chunk_setter_hpp
#define __hemlock_voxel_chunk_setter_hpp

#include "voxel/block.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;

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
        bool set_block(
            hmem::Handle<Chunk> chunk, BlockChunkPosition block_position, Block block
        );
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
        bool set_blocks(
            hmem::Handle<Chunk> chunk,
            BlockChunkPosition  start_block_position,
            BlockChunkPosition  end_block_position,
            Block               block
        );
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
        bool set_blocks(
            hmem::Handle<Chunk> chunk,
            BlockChunkPosition  start_block_position,
            BlockChunkPosition  end_block_position,
            Block*              blocks
        );

        template <typename DataType>
        void set_per_block_data(
            DataType*                buffer,
            hvox::BlockChunkPosition start_block_position,
            hvox::BlockChunkPosition end_block_position,
            DataType                 data
        );

        template <typename DataType>
        void set_per_block_data(
            DataType*                buffer,
            hvox::BlockChunkPosition start_block_position,
            hvox::BlockChunkPosition end_block_position,
            DataType*                data
        );
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/chunk/setter.inl"

#endif  // __hemlock_voxel_chunk_setter_hpp

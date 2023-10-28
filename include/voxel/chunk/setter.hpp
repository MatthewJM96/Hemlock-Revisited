#ifndef __hemlock_voxel_chunk_setter_hpp
#define __hemlock_voxel_chunk_setter_hpp

#include "voxel/chunk/components/core.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        /**
         * @brief Set the block at the given position in the given chunk.
         *
         * NOTE: This call is unsafe! Only call from a context where the corresponding
         * chunk entity is guaranteed to remain alive until the call returns.
         *
         * @param chunk The chunk in which to set the block.
         * @param position The position at which to set the block.
         * @param block The block to set.
         *
         * @return True if the block was set, false otherwise.
         */
        bool set_block(ChunkCore& chunk, BlockChunkPosition position, Block block);

        /**
         * @brief Set all points in a rectangular cuboid of the passed in chunk to a
         * specific block.
         *
         * NOTE: This call is unsafe! Only call from a context where the corresponding
         * chunk entity is guaranteed to remain alive until the call returns.
         *
         * @param chunk The chunk in which to set the block.
         * @param start The starting position of the range to set blocks for.
         * @param end The end position of the range to set blocks for.
         * @param block The block to set.
         *
         * @return True if the blocks were set, false otherwise.
         */
        bool set_blocks(
            ChunkCore&         chunk,
            BlockChunkPosition start,
            BlockChunkPosition end,
            Block              block
        );

        /**
         * @brief Set all points in a rectangular cuboid of the
         * passed in chunk to each block in a buffer. Note, the
         * buffer is assumed to go in x, then y, then z starting
         * from the near bottom left of the chunk.
         *
         * NOTE: This call is unsafe! Only call from a context where the corresponding
         * chunk entity is guaranteed to remain alive until the call returns.
         *
         * @param chunk The chunk in which to set the block.
         * @param start The starting position of the range to set blocks for.
         * @param end The end position of the range to set blocks for.
         * @param blocks The blocks to set.
         *
         * @return True if the blocks were set, false otherwise.
         */
        bool set_blocks(
            ChunkCore&         chunk,
            BlockChunkPosition start,
            BlockChunkPosition end,
            Block*             blocks
        );

        /**
         * @brief Sets elements of the given buffer that lie within the range of the
         * start and end block positions specified to the data value provided.
         *
         * NOTE: This call is unsafe! Only call from a context where the corresponding
         * buffer is guaranteed to remain alive until the call returns.
         *
         * @tparam DataType The type of the data to be set.
         * @param buffer The buffer to set data within.
         * @param start The starting position of the range to set data for.
         * @param end The end position of the range to set data for.
         * @param data The data to set.
         */
        template <typename DataType>
        void set_per_block_data(
            DataType*                buffer,
            hvox::BlockChunkPosition start,
            hvox::BlockChunkPosition end,
            DataType                 data
        );

        /**
         * @brief Sets elements of the given buffer that lie within the range of the
         * start and end block positions specified to the data value provided.
         *
         * NOTE: This call is unsafe! Only call from a context where the corresponding
         * buffer is guaranteed to remain alive until the call returns.
         *
         * @tparam DataType The type of the data to be set.
         * @param buffer The buffer to set data within.
         * @param start The starting position of the range to set data for.
         * @param end The end position of the range to set data for.
         * @param data Pointer to the data to set.
         */
        template <typename DataType>
        void set_per_block_data(
            DataType*                buffer,
            hvox::BlockChunkPosition start,
            hvox::BlockChunkPosition end,
            DataType*                data
        );
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/chunk/setter.inl"

#endif  // __hemlock_voxel_chunk_setter_hpp

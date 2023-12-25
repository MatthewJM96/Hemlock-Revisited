#ifndef __hemlock_voxel_chunk_setter_hpp
#define __hemlock_voxel_chunk_setter_hpp

#include "voxel/coordinate_system.h"
#include "voxel/state.hpp"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        /**
         * @brief Set the voxel at the given position in the given chunk.
         *
         * NOTE: This call is unsafe! Only call from a context where the corresponding
         * chunk entity is guaranteed to remain alive until the call returns.
         *
         * @param chunk The chunk in which to set the voxel.
         * @param position The position at which to set the voxel.
         * @param voxel The voxel to set.
         *
         * @return True if the voxel was set, false otherwise.
         */
        bool
        set_voxel(hmem::Handle<Chunk> chunk, VoxelChunkPosition position, Voxel voxel);

        /**
         * @brief Set all points in a rectangular cuboid of the passed in chunk to a
         * specific voxel.
         *
         * NOTE: This call is unsafe! Only call from a context where the corresponding
         * chunk entity is guaranteed to remain alive until the call returns.
         *
         * @param chunk The chunk in which to set the voxel.
         * @param start The starting position of the range to set voxels for.
         * @param end The end position of the range to set voxels for.
         * @param voxel The voxel to set.
         *
         * @return True if the voxels were set, false otherwise.
         */
        bool set_voxels(
            hmem::Handle<Chunk> chunk,
            VoxelChunkPosition  start,
            VoxelChunkPosition  end,
            Voxel               voxel
        );

        /**
         * @brief Set all points in a rectangular cuboid of the
         * passed in chunk to each voxel in a buffer. Note, the
         * buffer is assumed to go in x, then y, then z starting
         * from the near bottom left of the chunk.
         *
         * @param chunk The chunk in which to set the voxel.
         * @param start The starting position of the range to set voxels for.
         * @param end The end position of the range to set voxels for.
         * @param voxels The voxels to set.
         *
         * @return True if the voxels were set, false otherwise.
         */
        bool set_voxels(
            hmem::Handle<Chunk> chunk,
            VoxelChunkPosition  start,
            VoxelChunkPosition  end,
            Voxel*              voxels
        );

        /**
         * @brief Sets elements of the given buffer that lie within the range of the
         * start and end voxel positions specified to the data value provided.
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
        void set_per_voxel_data(
            DataType*                buffer,
            hvox::VoxelChunkPosition start,
            hvox::VoxelChunkPosition end,
            DataType                 data
        );

        /**
         * @brief Sets elements of the given buffer that lie within the range of the
         * start and end voxel positions specified to the data value provided.
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
        void set_per_voxel_data(
            DataType*                buffer,
            hvox::VoxelChunkPosition start,
            hvox::VoxelChunkPosition end,
            DataType*                data
        );
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/chunk/setter.inl"

#endif  // __hemlock_voxel_chunk_setter_hpp

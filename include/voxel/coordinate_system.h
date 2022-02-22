#ifndef __hemlock_voxel_coordinate_system_h
#define __hemlock_voxel_coordinate_system_h

namespace hemlock {
    namespace voxel {
        /// Spaces:
        // World-space: any space in which the player interacts directly.
        //              -> contains a grid-space of chunks, which themselves contain a chunk-space of blocks.
        // Grid-space:  discrete space with units of chunk width (i.e. each chunk is at an integer coordinate).
        //              -> used to reference locations of chunks in the world relative to one another.
        // Chunk-space: discrete space with units of block width (i.e. each block is at an integer coordinate).
        //              -> used to reference locations of blocks in a chunk relative to one another.

        /**
         * @brief Index into array of blocks that this block resides at.
         */
        using BlockIndex = ui32;

        /**
         * @brief Position of block withing chunk-space.
         */
        using BlockChunkPositionCoord = ui8;
        using BlockChunkPosition2D    = ui8v2;
        using BlockChunkPosition      = ui8v3;
        /**
         * @brief Position of block withing world-space.
         */
        using BlockWorldPositionCoord = i32;
        using BlockWorldPosition2D    = i32v2;
        using BlockWorldPosition      = i32v3;

        /**
         * @brief Unique ID of a column.
         */
        using ColumnID = ui64;
        /**
         * @brief Position of column withing world-space.
         */
        union ColumnWorldPosition {
            struct {
                i32 x, z;
            };
            ColumnID id;
        };

        /**
         * @brief Unique ID of a chunk.
         */
        using ChunkID = ui64;
        /**
         * @brief Position of chunk withing grid-space.
         */
        union ChunkGridPosition {
            struct {
                i64 x : 24; // 16777216 * CHUNK_WIDTH  blocks supported.
                i64 y : 16; //    65536 * CHUNK_HEIGHT blocks supported.
                i64 z : 24; // 16777216 * CHUNK_WIDTH  blocks supported.
            };
            ChunkID id;
        };

        /**
         * @brief Converts a block's chunk position into its index into the chunk's block array.
         * 
         * @param block_chunk_position The position of the block within the chunk's space.
         * @return ui32 The index of the block in the chunk's block array.
         */
        ui32 block_index(BlockChunkPosition block_chunk_position);

        /**
         * @brief Converts a block's world position into its chunk position.
         * 
         * @param block_world_position The position of the block within world space.
         * @return BlockChunkPosition The position of the block within the chunk's space.
         */
        BlockChunkPosition block_chunk_position(BlockWorldPosition block_world_position);

        /**
         * @brief Converts a block's index into the chunk's block array into its chunk position.
         * 
         * @param index The index of the block in the chunk's block array.
         * @return BlockChunkPosition The position of the block within the chunk's space.
         */
        BlockChunkPosition block_chunk_position(ui32 index);

        /**
         * @brief Converts a chunk grid position and block chunk position into a corresponding
         * world position.
         * 
         * @param chunk_grid_position The position of the chunk within grid space.
         * @param block_chunk_position The position of the block within chunk space.
         * @return BlockWorldPosition The position of the block within world space.
         */
        BlockWorldPosition block_world_position( ChunkGridPosition chunk_grid_position,
                                                BlockChunkPosition block_chunk_position = BlockChunkPosition{0.0f} );

        /**
         * @brief Converts a chunk grid position and block index into a corresponding
         * world position.
         * 
         * @param chunk_grid_position The position of the chunk within grid space.
         * @param index The index of the block in the chunk's block array.
         * @return BlockWorldPosition The position of the block within world space.
         */
        BlockWorldPosition block_world_position(ChunkGridPosition chunk_grid_position, ui32 index);
    }
}
namespace hvox = hemlock::voxel;

bool operator==(hvox::ColumnWorldPosition lhs, hvox::ColumnWorldPosition rhs);
bool operator!=(hvox::ColumnWorldPosition lhs, hvox::ColumnWorldPosition rhs);

bool operator==(hvox::ChunkGridPosition lhs, hvox::ChunkGridPosition rhs);
bool operator!=(hvox::ChunkGridPosition lhs, hvox::ChunkGridPosition rhs);

namespace std {
    template <>
    struct hash<hvox::ColumnWorldPosition> {
        std::size_t operator()(hvox::ColumnWorldPosition cwp) const {
            std::hash<hvox::ColumnID> hash;
            return hash(cwp.id);
        }
    };
    template <>
    struct hash<hvox::ChunkGridPosition> {
        std::size_t operator()(hvox::ChunkGridPosition cwp) const {
            std::hash<hvox::ChunkID> hash;
            return hash(cwp.id);
        }
    };
}

#endif // __hemlock_voxel_coordinate_system_h

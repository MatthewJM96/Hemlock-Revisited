#ifndef __hemlock_voxel_coordinate_system_h
#define __hemlock_voxel_coordinate_system_h

namespace hemlock {
    namespace voxel {
        /// Spaces:
        // World-space: any space in which the player interacts directly.
        //              -> contains a grid-space of chunks, which themselves contain a
        //              chunk-space of voxels.
        // Grid-space:  discrete space with units of chunk width (i.e. each chunk is
        // at an integer coordinate).
        //              -> used to reference locations of chunks in the world relative
        //              to one another.
        // Chunk-space: discrete space with units of voxel width (i.e. each voxel is
        // at an integer coordinate).
        //              -> used to reference locations of voxels in a chunk relative
        //              to one another.

        /**
         * @brief Index into array of voxels that this voxel resides at.
         */
        using VoxelIndex = ui32;

        /**
         * @brief Position of voxel withing chunk-space.
         */
        using VoxelChunkPositionCoord = ui8;
        using VoxelChunkPosition2D    = ui8v2;
        using VoxelChunkPosition      = ui8v3;
        /**
         * @brief Position of voxel withing world-space.
         */
        using VoxelWorldPositionCoord = i32;
        using VoxelWorldPosition2D    = i32v2;
        using VoxelWorldPosition      = i32v3;

        /**
         * @brief Position of entity in world-space.
         * Note that these values are 32/32 fixed-point
         * numbers.
         */
        using EntityWorldPositionCoord = i64;
        using EntityWorldPosition2D    = i64v2;
        using EntityWorldPosition      = i64v3;

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
            HEMLOCK_PACKED_STRUCT(struct {
                i64 x : 24;  // 16777216 * CHUNK_WIDTH  voxels supported.
                i64 y : 16;  //    65536 * CHUNK_HEIGHT voxels supported.
                i64 z : 24;  // 16777216 * CHUNK_WIDTH  voxels supported.
            });

            ChunkID id;
        };

        /**
         * @brief Converts a voxel's chunk position into its index into the chunk's
         * voxel array.
         *
         * @param voxel_chunk_position The position of the voxel within the chunk's
         * space.
         * @return ui32 The index of the voxel in the chunk's voxel array.
         */
        ui32 voxel_index(VoxelChunkPosition voxel_chunk_position);

        /**
         * @brief Converts a voxel's world position into its chunk position.
         *
         * @param voxel_world_position The position of the voxel within world space.
         * @return VoxelChunkPosition The position of the voxel within the chunk's
         * space.
         */
        VoxelChunkPosition voxel_chunk_position(VoxelWorldPosition voxel_world_position
        );

        /**
         * @brief Converts a voxel's index into the chunk's voxel array into its chunk
         * position.
         *
         * @param index The index of the voxel in the chunk's voxel array.
         * @return VoxelChunkPosition The position of the voxel within the chunk's
         * space.
         */
        VoxelChunkPosition voxel_chunk_position(ui32 index);

        /**
         * @brief Converts a general point to the voxel coordinate in which it can be
         * found.
         *
         * @param position The general position to locate in voxel coordinates.
         * @return VoxelWorldPosition The voxel inside which a general position is
         * located.
         */
        VoxelWorldPosition voxel_world_position(f32v3 position);

        /**
         * @brief Converts a chunk grid position and voxel chunk position into a
         * corresponding world position.
         *
         * @param chunk_grid_position The position of the chunk within grid space.
         * @param voxel_chunk_position The position of the voxel within chunk space.
         * @return VoxelWorldPosition The position of the voxel within world space.
         */
        VoxelWorldPosition voxel_world_position(
            ChunkGridPosition  chunk_grid_position,
            VoxelChunkPosition voxel_chunk_position = VoxelChunkPosition{ 0 }
        );

        /**
         * @brief Converts a chunk grid position and voxel index into a corresponding
         * world position.
         *
         * @param chunk_grid_position The position of the chunk within grid space.
         * @param index The index of the voxel in the chunk's voxel array.
         * @return VoxelWorldPosition The position of the voxel within world space.
         */
        VoxelWorldPosition
        voxel_world_position(ChunkGridPosition chunk_grid_position, ui32 index);

        /**
         * @brief Converts a voxel's world position into the grid position of the
         * chunk in which it exists.
         *
         * @param voxel_world_position The world position of the voxel.
         * @return ChunkGridPosition The grid position of the enclosing chunk.
         */
        ChunkGridPosition chunk_grid_position(VoxelWorldPosition voxel_world_position);
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

bool operator==(hvox::ColumnWorldPosition lhs, hvox::ColumnWorldPosition rhs);
bool operator!=(hvox::ColumnWorldPosition lhs, hvox::ColumnWorldPosition rhs);

bool operator==(hvox::ChunkGridPosition lhs, hvox::ChunkGridPosition rhs);
bool operator!=(hvox::ChunkGridPosition lhs, hvox::ChunkGridPosition rhs);

namespace std {
    template <>
    struct hash<hvox::ColumnWorldPosition> {
        std::size_t operator()(hvox::ColumnWorldPosition cwp) const {
            std::hash<hvox::ColumnID> _hash;
            return _hash(cwp.id);
        }
    };

    template <>
    struct hash<hvox::ChunkGridPosition> {
        std::size_t operator()(hvox::ChunkGridPosition cwp) const {
            std::hash<hvox::ChunkID> _hash;
            return _hash(cwp.id);
        }
    };
}  // namespace std

#endif  // __hemlock_voxel_coordinate_system_h

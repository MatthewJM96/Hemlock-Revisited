#include "stdafx.h"

#include "voxel/chunk/chunk.h"

#include "voxel/coordinate_system.h"

ui32 hvox::voxel_index(VoxelChunkPosition voxel_chunk_position) {
    return static_cast<ui32>(voxel_chunk_position.x)
           + static_cast<ui32>(voxel_chunk_position.y) * CHUNK_LENGTH
           + static_cast<ui32>(voxel_chunk_position.z) * CHUNK_AREA;
}

hvox::VoxelChunkPosition
hvox::voxel_chunk_position(VoxelWorldPosition voxel_world_position) {
    return { static_cast<VoxelChunkPositionCoord>(
                 (voxel_world_position.x % CHUNK_LENGTH + CHUNK_LENGTH) % CHUNK_LENGTH
             ),
             static_cast<VoxelChunkPositionCoord>(
                 (voxel_world_position.y % CHUNK_LENGTH + CHUNK_LENGTH) % CHUNK_LENGTH
             ),
             static_cast<VoxelChunkPositionCoord>(
                 (voxel_world_position.z % CHUNK_LENGTH + CHUNK_LENGTH) % CHUNK_LENGTH
             ) };
}

hvox::VoxelChunkPosition hvox::voxel_chunk_position(ui32 index) {
    VoxelChunkPositionCoord z = static_cast<VoxelChunkPositionCoord>(
        static_cast<f32>(index) / static_cast<f32>(CHUNK_AREA)
    );

    index %= CHUNK_AREA;

    VoxelChunkPositionCoord y = static_cast<VoxelChunkPositionCoord>(
        static_cast<f32>(index) / static_cast<f32>(CHUNK_LENGTH)
    );

    VoxelChunkPositionCoord x
        = static_cast<VoxelChunkPositionCoord>(index % CHUNK_LENGTH);

    return { x, y, z };
}

hvox::VoxelWorldPosition hvox::voxel_world_position(f32v3 position) {
    return { static_cast<VoxelWorldPositionCoord>(glm::floor(position.x)),
             static_cast<VoxelWorldPositionCoord>(glm::floor(position.y)),
             static_cast<VoxelWorldPositionCoord>(glm::floor(position.z)) };
}

hvox::VoxelWorldPosition hvox::voxel_world_position(
    ChunkGridPosition  chunk_grid_position,
    VoxelChunkPosition voxel_chunk_position /*= VoxelChunkPosition{0.0f}*/
) {
    return {
        static_cast<VoxelWorldPositionCoord>(chunk_grid_position.x) * CHUNK_LENGTH
            + voxel_chunk_position.x,
        static_cast<VoxelWorldPositionCoord>(chunk_grid_position.y) * CHUNK_LENGTH
            + voxel_chunk_position.y,
        static_cast<VoxelWorldPositionCoord>(chunk_grid_position.z) * CHUNK_LENGTH
            + voxel_chunk_position.z,
    };
}

hvox::VoxelWorldPosition
hvox::voxel_world_position(ChunkGridPosition chunk_grid_position, ui32 index) {
    return voxel_world_position(chunk_grid_position, voxel_chunk_position(index));
}

hvox::ChunkGridPosition
hvox::chunk_grid_position(VoxelWorldPosition voxel_world_position) {
    return {
        {static_cast<i64>(glm::floor(
static_cast<f32>(voxel_world_position.x) / static_cast<f32>(CHUNK_LENGTH)
)),
         static_cast<i64>(glm::floor(
         static_cast<f32>(voxel_world_position.y) / static_cast<f32>(CHUNK_LENGTH)
         )),
         static_cast<i64>(glm::floor(
         static_cast<f32>(voxel_world_position.z) / static_cast<f32>(CHUNK_LENGTH)
         ))}
    };
}

bool operator==(hvox::ColumnWorldPosition lhs, hvox::ColumnWorldPosition rhs) {
    return lhs.id == rhs.id;
}

bool operator!=(hvox::ColumnWorldPosition lhs, hvox::ColumnWorldPosition rhs) {
    return !(lhs == rhs);
}

bool operator==(hvox::ChunkGridPosition lhs, hvox::ChunkGridPosition rhs) {
    return lhs.id == rhs.id;
}

bool operator!=(hvox::ChunkGridPosition lhs, hvox::ChunkGridPosition rhs) {
    return !(lhs == rhs);
}

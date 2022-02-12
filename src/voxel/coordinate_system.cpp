#include "stdafx.h"

#include "voxel/chunk.hpp"

#include "voxel/coordinate_system.h"

ui32 hvox::block_index(BlockChunkPosition block_chunk_position) {
    return static_cast<ui32>(block_chunk_position.x)
            + static_cast<ui32>(block_chunk_position.y) * CHUNK_SIZE
            + static_cast<ui32>(block_chunk_position.z) * CHUNK_SIZE * CHUNK_SIZE;
}

hvox::BlockChunkPosition hvox::block_chunk_position(BlockWorldPosition block_world_position) {
    return {
        static_cast<BlockChunkPositionCoord>(block_world_position.x % CHUNK_SIZE),
        static_cast<BlockChunkPositionCoord>(block_world_position.y % CHUNK_SIZE),
        static_cast<BlockChunkPositionCoord>(block_world_position.z % CHUNK_SIZE)
    };
}

hvox::BlockChunkPosition hvox::block_chunk_position(ui32 index) {
    BlockChunkPositionCoord z =
        static_cast<BlockChunkPositionCoord>(
            glm::floor(static_cast<f32>(index) / static_cast<f32>(CHUNK_SIZE * CHUNK_SIZE))
        );

    index %= CHUNK_SIZE * CHUNK_SIZE;

    BlockChunkPositionCoord y =
        static_cast<BlockChunkPositionCoord>(
            glm::floor(static_cast<f32>(index) / static_cast<f32>(CHUNK_SIZE))
        );

    BlockChunkPositionCoord x = static_cast<BlockChunkPositionCoord>(index % CHUNK_SIZE);

    return { x, y, z };
}

hvox::BlockWorldPosition hvox::block_world_position( ChunkGridPosition chunk_grid_position,
                                              BlockChunkPosition block_chunk_position /*= BlockChunkPosition{0.0f}*/ ) {
    return {
        static_cast<BlockWorldPositionCoord>(chunk_grid_position.x) * CHUNK_SIZE + block_chunk_position.x,
        static_cast<BlockWorldPositionCoord>(chunk_grid_position.y) * CHUNK_SIZE + block_chunk_position.y,
        static_cast<BlockWorldPositionCoord>(chunk_grid_position.z) * CHUNK_SIZE + block_chunk_position.z,
    };
}

hvox::BlockWorldPosition hvox::block_world_position(ChunkGridPosition chunk_grid_position, ui32 index) {
    return block_world_position(chunk_grid_position, block_chunk_position(index));
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

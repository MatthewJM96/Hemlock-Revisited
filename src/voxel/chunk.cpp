#include "stdafx.h"

#include "voxel/block.hpp"

#include "voxel/chunk.h"

hvox::Chunk::Chunk() :
    neighbours(NULL_NEIGHBOURS),
    blocks(nullptr),
    m_owns_blocks(true)
{ /* Empty. */ }

void hvox::Chunk::init() {
    blocks = new Block[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    std::fill_n(blocks, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, Block{ false });

    neighbours = NULL_NEIGHBOURS;
}

void hvox::Chunk::init(Block* _blocks) {
    blocks = _blocks;
    m_owns_blocks = false;

    neighbours = NULL_NEIGHBOURS;
}

void hvox::Chunk::dispose() {
    if (m_owns_blocks) delete[] blocks;
    blocks = nullptr;

    neighbours = NULL_NEIGHBOURS;
}

void hvox::Chunk::update(TimeData) {
    if (state == ChunkState::MESHED) {
        if (hg::upload_mesh(mesh, mesh_handles, hg::MeshDataVolatility::STATIC))
            state = ChunkState::MESH_UPLOADED;
    }
}

bool hvox::set_block( Chunk* chunk,
          BlockChunkPosition block_position,
                       Block block )
{
    auto block_idx = block_index(block_position);

    if (!chunk->gen_task_active) {
        bool should_cancel = chunk->on_block_change({
            chunk,
            chunk->blocks[block_idx],
            block,
            block_position
        });
        if (should_cancel) return false;
    }

    chunk->blocks[block_idx] = block;

    return true;
}

bool hvox::set_blocks( Chunk* chunk,
           BlockChunkPosition start_block_position,
           BlockChunkPosition end_block_position,
                        Block block )
{
    if (!chunk->gen_task_active) {
        // bool should_cancel = chunk->on_bulk_block_change({
        //     chunk,
        //     &block,
        //     true,
        //     start_block_position,
        //     end_block_position
        // });
        // if (should_cancel) return false;
    }

    /*
     * If we span the whole chunk, we just fill the whole buffer.
     */
    if (start_block_position == BlockChunkPosition{0} && end_block_position == BlockChunkPosition{CHUNK_SIZE}) {
        std::fill_n(chunk->blocks, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, block);
    /*
     * If we span the XY plane, then we set the whole cuboid as all
     * elements it touches are contiguous in memory.
     */
    } else if (start_block_position.xy() == BlockChunkPosition2D{0} && end_block_position.xy() == BlockChunkPosition2D{CHUNK_SIZE}) {
        auto batch_size = CHUNK_SIZE * CHUNK_SIZE * (end_block_position.z - start_block_position.z);
        auto start_idx = block_index({
            0, 0, start_block_position.z
        });
        std::fill_n(&(chunk->blocks[start_idx]), batch_size, block);
    /*
     * If we span the X line, then we set squares in XY plane one at a time.
     */
    } else if (start_block_position.x == 0 && end_block_position.x == CHUNK_SIZE) {
        auto batch_size = CHUNK_SIZE * (end_block_position.y - start_block_position.y);
        for (auto z = start_block_position.z; z < end_block_position.z; ++z) {
            auto start_idx = block_index({
                0, start_block_position.y, z
            });
            std::fill_n(&(chunk->blocks[start_idx]), batch_size, block);
        }
    /*
     * This is a worst-case bulk setting, we have to set each X segment
     * one at a time.
     */
    } else {
        auto batch_size = end_block_position.x - start_block_position.x;
        for (auto z = start_block_position.z; z < end_block_position.z; ++z) {
            for (auto y = start_block_position.y; y < end_block_position.y; ++y) {
                auto start_idx = block_index({
                    start_block_position.x, y, z
                });
                std::fill_n(&(chunk->blocks[start_idx]), batch_size, block);
            }
        }
    }

    return true;
}

bool hvox::set_blocks( Chunk* chunk,
           BlockChunkPosition start_block_position,
           BlockChunkPosition end_block_position,
                       Block* blocks )
{
    if (!chunk->gen_task_active) {
        bool should_cancel = chunk->on_bulk_block_change({
            chunk,
            blocks,
            false,
            start_block_position,
            end_block_position
        });
        if (should_cancel) return false;
    }

    /*
     * If we span the whole chunk, we just copy in the whole buffer.
     */
    if (start_block_position == BlockChunkPosition{0} && end_block_position == BlockChunkPosition{CHUNK_SIZE}) {
        std::memcpy(chunk->blocks, blocks, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    /*
     * If we span the XY plane, then we copy the whole cuboid as all
     * elements it touches are contiguous in memory.
     */
    } else if (start_block_position.xy() == BlockChunkPosition2D{0} && end_block_position.xy() == BlockChunkPosition2D{CHUNK_SIZE}) {
        auto batch_size = CHUNK_SIZE * CHUNK_SIZE * (end_block_position.z - start_block_position.z);
        auto start_idx = block_index({
            0, 0, start_block_position.z
        });
        std::memcpy(&(chunk->blocks[start_idx]), blocks, batch_size);
    /*
     * If we span the X line, then we copy squares in XY plane one at a time.
     */
    } else if (start_block_position.x == 0 && end_block_position.x == CHUNK_SIZE) {
        auto batch_size = CHUNK_SIZE * (end_block_position.y - start_block_position.y);
        for (auto z = start_block_position.z; z < end_block_position.z; ++z) {
            auto chunk_blocks_start_idx = block_index({
                0, start_block_position.y, z
            });
            auto new_blocks_start_idx = batch_size * (z - start_block_position.z);
            std::memcpy(
                &(chunk->blocks[chunk_blocks_start_idx]),
                &blocks[new_blocks_start_idx],
                batch_size
            );
        }
    /*
     * This is a worst-case bulk setting, we have to copy each X segment
     * one at a time.
     */
    } else {
        auto batch_size = end_block_position.x - start_block_position.x;
        for (auto z = start_block_position.z; z < end_block_position.z; ++z) {
            for (auto y = start_block_position.y; y < end_block_position.y; ++y) {
                auto chunk_blocks_start_idx = block_index({
                    start_block_position.x, y, z
                });
                auto new_blocks_start_idx =
                        batch_size * (y - start_block_position.y)
                            + batch_size * (end_block_position.y - start_block_position.y) * (z - start_block_position.z);
                std::memcpy(
                    &(chunk->blocks[chunk_blocks_start_idx]),
                    &blocks[new_blocks_start_idx],
                    batch_size
                );
            }
        }
    }

    return true;
}
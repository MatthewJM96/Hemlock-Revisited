#include "stdafx.h"

#include "voxel/chunk/chunk.h"

#include "voxel/chunk/setter.hpp"

bool hvox::set_block(
    hmem::Handle<Chunk> chunk, BlockChunkPosition position, Block block
) {
    auto block_idx = block_index(position);

    {
        std::shared_lock<std::shared_mutex> lock;
        auto                                blocks = chunk->blocks.get(lock);

        bool gen_task_active
            = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
        if (!gen_task_active) {
            bool should_cancel
                = chunk->on_block_change({ blocks[block_idx], block, position });
            if (should_cancel) return false;
        }
    }

    std::unique_lock<std::shared_mutex> lock;
    auto                                blocks = chunk->blocks.get(lock);

    blocks[block_idx] = block;

    return true;
}

bool hvox::set_blocks(
    hmem::Handle<Chunk> chunk,
    BlockChunkPosition  start,
    BlockChunkPosition  end,
    Block               block
) {
    {
        bool gen_task_active
            = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
        if (!gen_task_active) {
            bool should_cancel
                = chunk->on_bulk_block_change({ &block, true, start, end });
            if (should_cancel) return false;
        }
    }

    std::unique_lock<std::shared_mutex> lock;
    auto                                chunk_blocks = chunk->blocks.get(lock);

    set_per_block_data(chunk_blocks, start, end, block);

    return true;
}

bool hvox::set_blocks(
    hmem::Handle<Chunk> chunk,
    BlockChunkPosition  start,
    BlockChunkPosition  end,
    Block*              blocks
) {
    {
        bool gen_task_active
            = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
        if (!gen_task_active) {
            bool should_cancel
                = chunk->on_bulk_block_change({ blocks, false, start, end });
            if (should_cancel) return false;
        }
    }

    std::unique_lock<std::shared_mutex> lock;
    auto                                chunk_blocks = chunk->blocks.get(lock);

    set_per_block_data(chunk_blocks, start, end, blocks);

    return true;
}

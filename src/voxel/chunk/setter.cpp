#include "stdafx.h"

#include "voxel/chunk/chunk.h"

#include "voxel/chunk/setter.hpp"

bool hvox::set_block(
    hmem::Handle<Chunk> chunk, BlockChunkPosition position, Block block
) {
    auto block_idx = block_index(position);

    {
        std::shared_lock lock(chunk->blocks_mutex);

        bool gen_task_active
            = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
        if (!gen_task_active) {
            bool should_cancel = chunk->on_block_change(
                { chunk, chunk->blocks[block_idx], block, position }
            );
            if (should_cancel) return false;
        }
    }

    std::lock_guard lock(chunk->blocks_mutex);

    chunk->blocks[block_idx] = block;

    return true;
}

bool hvox::set_blocks(
    hmem::Handle<Chunk> chunk,
    BlockChunkPosition  start,
    BlockChunkPosition  end,
    Block               block
) {
    {
        std::shared_lock lock(chunk->blocks_mutex);

        bool gen_task_active
            = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
        if (!gen_task_active) {
            bool should_cancel
                = chunk->on_bulk_block_change({ chunk, &block, true, start, end });
            if (should_cancel) return false;
        }
    }

    std::lock_guard lock(chunk->blocks_mutex);

    set_per_block_data(chunk->blocks, start, end, block);

    return true;
}

bool hvox::set_blocks(
    hmem::Handle<Chunk> chunk,
    BlockChunkPosition  start,
    BlockChunkPosition  end,
    Block*              blocks
) {
    {
        std::shared_lock lock(chunk->blocks_mutex);

        bool gen_task_active
            = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
        if (!gen_task_active) {
            bool should_cancel
                = chunk->on_bulk_block_change({ chunk, blocks, false, start, end });
            if (should_cancel) return false;
        }
    }

    std::lock_guard lock(chunk->blocks_mutex);

    set_per_block_data(chunk->blocks, start, end, blocks);

    return true;
}

#include "stdafx.h"

#include "voxel/chunk/chunk.h"

#include "voxel/chunk/setter.hpp"

bool hvox::set_voxel(
    hmem::Handle<Chunk> chunk, VoxelChunkPosition position, Voxel voxel
) {
    auto voxel_idx = voxel_index(position);

    {
        std::shared_lock<std::shared_mutex> lock;
        auto                                voxels = chunk->voxels.get(lock);

        bool gen_task_active
            = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
        if (!gen_task_active) {
            bool should_cancel
                = chunk->on_voxel_change({ voxels[voxel_idx], voxel, position });
            if (should_cancel) return false;
        }
    }

    std::unique_lock<std::shared_mutex> lock;
    auto                                voxels = chunk->voxels.get(lock);

    voxels[voxel_idx] = voxel;

    return true;
}

bool hvox::set_voxels(
    hmem::Handle<Chunk> chunk,
    VoxelChunkPosition  start,
    VoxelChunkPosition  end,
    Voxel               voxel
) {
    {
        bool gen_task_active
            = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
        if (!gen_task_active) {
            bool should_cancel
                = chunk->on_bulk_voxel_change({ &voxel, true, start, end });
            if (should_cancel) return false;
        }
    }

    std::unique_lock<std::shared_mutex> lock;
    auto                                chunk_voxels = chunk->voxels.get(lock);

    set_per_voxel_data(chunk_voxels, start, end, voxel);

    return true;
}

bool hvox::set_voxels(
    hmem::Handle<Chunk> chunk,
    VoxelChunkPosition  start,
    VoxelChunkPosition  end,
    Voxel*              voxels
) {
    {
        bool gen_task_active
            = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
        if (!gen_task_active) {
            bool should_cancel
                = chunk->on_bulk_voxel_change({ voxels, false, start, end });
            if (should_cancel) return false;
        }
    }

    std::unique_lock<std::shared_mutex> lock;
    auto                                chunk_voxels = chunk->voxels.get(lock);

    set_per_voxel_data(chunk_voxels, start, end, voxels);

    return true;
}

#include "stdafx.h"

#include "voxel/block.hpp"

#include "voxel/chunk.h"

hvox::Chunk::Chunk() :
    neighbours({}),
    lod_level(0),
    generation(ChunkState::NONE),
    meshing(ChunkState::NONE),
    mesh_uploading(ChunkState::NONE),
    navmeshing(ChunkState::NONE),
    navmesh_stitch{ ChunkState::NONE, ChunkState::NONE, ChunkState::NONE,
                    ChunkState::NONE, ChunkState::NONE, ChunkState::NONE,
                    ChunkState::NONE, ChunkState::NONE, ChunkState::NONE,
                    ChunkState::NONE, ChunkState::NONE } { /* Empty. */
}

hvox::Chunk::~Chunk() {
    // debug_printf("Unloading chunk at (%d, %d, %d).\n", position.x, position.y,
    // position.z);

    blocks.dispose();
    mesh.dispose();
    navmesh.dispose();

    neighbours = {};
}

void hvox::Chunk::init(
    hmem::WeakHandle<Chunk>             self,
    hmem::Handle<ChunkBlockPager>       block_pager,
    hmem::Handle<ChunkMeshPager>        mesh_pager,
    hmem::Handle<ai::ChunkNavmeshPager> navmesh_pager
) {
    init_events(self);

    blocks.init(block_pager);
    blocks.generate_buffer();
    mesh.init(mesh_pager);
    mesh.generate_buffer();
    navmesh.init(navmesh_pager);
    navmesh.generate_buffer();

    neighbours = {};
}

void hvox::Chunk::update(FrameTime) {
    // Empty for now.
}

void hvox::Chunk::init_events(hmem::WeakHandle<Chunk> self) {
    on_block_change.set_sender(Sender(self));
    on_bulk_block_change.set_sender(Sender(self));
    on_load.set_sender(Sender(self));
    on_mesh_change.set_sender(Sender(self));
    on_navmesh_change.set_sender(Sender(self));
    on_lod_change.set_sender(Sender(self));
    on_unload.set_sender(Sender(self));
}

bool hvox::set_block(
    hmem::Handle<Chunk> chunk, BlockChunkPosition block_position, Block block
) {
    hmem::UniqueResourceLock lock;
    auto                     blocks = chunk->blocks.get(lock);

    auto block_idx = block_index(block_position);

    bool gen_task_active
        = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
    if (!gen_task_active) {
        bool should_cancel = chunk->on_block_change(
            { chunk, blocks.data[block_idx], block, block_position }
        );
        if (should_cancel) return false;
    }

    blocks.data[block_idx] = block;

    return true;
}

bool hvox::set_blocks(
    hmem::Handle<Chunk> chunk,
    BlockChunkPosition  start_block_position,
    BlockChunkPosition  end_block_position,
    Block               block
) {
    hmem::UniqueResourceLock lock;
    auto                     chunk_blocks = chunk->blocks.get(lock);

    bool gen_task_active
        = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
    if (!gen_task_active) {
        bool should_cancel = chunk->on_bulk_block_change(
            { chunk, &block, true, start_block_position, end_block_position }
        );
        if (should_cancel) return false;
    }

    set_per_block_data(
        chunk_blocks.data, start_block_position, end_block_position, block
    );

    return true;
}

bool hvox::set_blocks(
    hmem::Handle<Chunk> chunk,
    BlockChunkPosition  start_block_position,
    BlockChunkPosition  end_block_position,
    Block*              blocks
) {
    hmem::UniqueResourceLock lock;
    auto                     chunk_blocks = chunk->blocks.get(lock);

    bool gen_task_active
        = chunk->generation.load(std::memory_order_acquire) == ChunkState::ACTIVE;
    if (!gen_task_active) {
        // TODO(Matthew): this is a problem... don't really want to provide chunk blocks
        //                directly nor do we want deadlocks if people try to access
        //                write on them from event triggers.
        bool should_cancel = chunk->on_bulk_block_change({ chunk,
                                                           chunk_blocks.data,
                                                           false,
                                                           start_block_position,
                                                           end_block_position });
        if (should_cancel) return false;
    }

    set_per_block_data(
        chunk_blocks.data, start_block_position, end_block_position, blocks
    );

    return true;
}

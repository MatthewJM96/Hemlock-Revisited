#include "stdafx.h"

#include "voxel/block.hpp"

#include "voxel/chunk/chunk.h"

hvox::Chunk::Chunk() :
    neighbours({}),
    blocks(nullptr),
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

    if (blocks) m_block_pager->free_page(blocks);
    blocks = nullptr;

    instance.dispose();

    neighbours = {};
}

void hvox::Chunk::init(
    hmem::WeakHandle<Chunk>              self,
    hmem::Handle<ChunkBlockPager>        block_pager,
    hmem::Handle<ChunkInstanceDataPager> instance_data_pager
) {
    init_events(self);

    blocks        = block_pager->get_page();
    m_block_pager = block_pager;

    instance.init(instance_data_pager);

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

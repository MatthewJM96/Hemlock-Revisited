#include "stdafx.h"

#include "voxel/block.hpp"

#include "voxel/chunk.h"

hvox::Chunk::Chunk() :
    neighbours({}),
    blocks(nullptr),
    state(ChunkState::NONE),
    pending_task(ChunkLoadTaskKind::NONE),
    unload(false),
    m_owns_blocks(true)
{ /* Empty. */ }

hvox::Chunk::~Chunk() {
    dispose();
}

void hvox::Chunk::init(hmem::WeakHandle<Chunk> self) {
    init_events(self);

    blocks = new Block[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    std::fill_n(blocks, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, Block{ false });

    neighbours = {};

    state.store(ChunkState::PRELOADED, std::memory_order_release);
}

void hvox::Chunk::init(hmem::WeakHandle<Chunk> self, Block* _blocks) {
    init_events(self);

    blocks = _blocks;
    m_owns_blocks = false;

    neighbours = {};

    state.store(ChunkState::PRELOADED, std::memory_order_release);
}

void hvox::Chunk::dispose() {
    if (m_owns_blocks) delete[] blocks;
    blocks = nullptr;

    neighbours = {};
}

void hvox::Chunk::update(TimeData) {
    // Empty for now.
}

void hvox::Chunk::init_events(hmem::WeakHandle<Chunk> self) {
    on_block_change         .set_sender(Sender(self));
    on_bulk_block_change    .set_sender(Sender(self));
    on_mesh_change          .set_sender(Sender(self));
    on_render_state_change  .set_sender(Sender(self));
    on_unload               .set_sender(Sender(self));
}

bool hvox::set_block( hmem::Handle<Chunk> chunk,
                       BlockChunkPosition block_position,
                                    Block block )
{
    auto block_idx = block_index(block_position);

    bool gen_task_active = chunk->gen_task_active.load(std::memory_order_acquire);
    if (!gen_task_active) {
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

bool hvox::set_blocks( hmem::Handle<Chunk> chunk,
                        BlockChunkPosition start_block_position,
                        BlockChunkPosition end_block_position,
                                     Block block )
{
    bool gen_task_active = chunk->gen_task_active.load(std::memory_order_acquire);
    if (!gen_task_active) {
        bool should_cancel = chunk->on_bulk_block_change({
            chunk,
            &block,
            true,
            start_block_position,
            end_block_position
        });
        if (should_cancel) return false;
    }

    set_per_block_data(
        chunk->blocks,
        start_block_position,
        end_block_position,
        block
    );

    return true;
}

bool hvox::set_blocks( hmem::Handle<Chunk> chunk,
                        BlockChunkPosition start_block_position,
                        BlockChunkPosition end_block_position,
                                    Block* blocks )
{
    bool gen_task_active = chunk->gen_task_active.load(std::memory_order_acquire);
    if (!gen_task_active) {
        bool should_cancel = chunk->on_bulk_block_change({
            chunk,
            blocks,
            false,
            start_block_position,
            end_block_position
        });
        if (should_cancel) return false;
    }

    set_per_block_data(
        chunk->blocks,
        start_block_position,
        end_block_position,
        blocks
    );

    return true;
}
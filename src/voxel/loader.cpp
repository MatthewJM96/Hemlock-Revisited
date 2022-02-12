#include "stdafx.h"

#include "voxel/block.hpp"
#include "voxel/chunk.h"

#include "voxel/loader.h"

void hvox::ChunkLoadTask::execute(ChunkGenThreadState*, ChunkGenTaskQueue*) {
    BlockWorldPosition chunk_position = block_world_position(m_chunk->position, 0);

    if (chunk_position.y < 0) {
        set_blocks(m_chunk, BlockChunkPosition{0}, BlockChunkPosition{CHUNK_SIZE}, Block{1});
    }

    // for (auto x = 0; x < CHUNK_SIZE; ++x) {
    //     for (auto z = 0; z < CHUNK_SIZE; ++z) {
    //         // Do stuff...
    //     }
    // }
}

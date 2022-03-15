#include "stdafx.h"

#include "voxel/chunk/allocator.h"

void hvox::ChunkAllocator::dispose() {
    ChunkHandleMetadatas().swap(m_metadata);
}

hvox::ChunkHandle& hvox::ChunkAllocator::acquire(ChunkGridPosition pos) {
    return ChunkAllocator::acquire(pos.id);
}

hvox::ChunkHandle& hvox::ChunkAllocator::acquire(ChunkID id) {

}

bool hvox::ChunkAllocator::release(ChunkHandle&& handle) {

}

hvox::ChunkHandle& hvox::ChunkAllocator::acquire_existing(ChunkID id) {

}

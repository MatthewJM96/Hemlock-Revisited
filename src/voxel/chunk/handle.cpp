#include "stdafx.h"

#include "voxel/chunk.h"

#include "voxel/chunk/handle.h"

hvox::ChunkHandleMetadatas hvox::ChunkHandle::metadata = {};

hvox::ChunkHandle::ChunkHandle() :
    m_chunk(nullptr)
{
    // Empty.
}

hvox::ChunkHandle::ChunkHandle(const ChunkHandle& rhs) {
    *this = std::move(ChunkHandle::acquire_existing(rhs->position.id));
}

hvox::ChunkHandle& hvox::ChunkHandle::operator=(const ChunkHandle& rhs) {
    *this = std::move(ChunkHandle::acquire_existing(rhs->position.id));

    return *this;
}

hvox::ChunkHandle::ChunkHandle(ChunkHandle&& rhs) :
    m_chunk(rhs.m_chunk)
{
    rhs.m_chunk = nullptr;
}

hvox::ChunkHandle& hvox::ChunkHandle::operator=(ChunkHandle&& rhs) {
    m_chunk = rhs.m_chunk;

    rhs.m_chunk = nullptr;

    return *this;
}

hvox::Chunk& hvox::ChunkHandle::operator*() {
    return *m_chunk;
}

const hvox::Chunk& hvox::ChunkHandle::operator*() const {
    return *m_chunk;
}

hvox::Chunk* hvox::ChunkHandle::operator->() {
    return m_chunk;
}

const hvox::Chunk* hvox::ChunkHandle::operator->() const {
    return m_chunk;
}

bool hvox::ChunkHandle::operator==(void* possible_nullptr) {
    return (possible_nullptr == nullptr) && (m_chunk == nullptr);
}

bool hvox::ChunkHandle::operator==(const ChunkHandle& handle) {
    return m_chunk == handle.m_chunk;
}

bool hvox::ChunkHandle::operator!=(void* possible_nullptr) {
    return !(*this == possible_nullptr);
}

bool hvox::ChunkHandle::operator!=(const ChunkHandle& handle) {
    return !(*this == handle);
}

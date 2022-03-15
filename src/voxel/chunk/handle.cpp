#include "stdafx.h"

#include "voxel/chunk.h"

#include "voxel/chunk/handle.h"

hvox::ChunkHandle::ChunkHandle() :
    m_chunk(nullptr)
{
    // Empty.
}

hvox::ChunkHandle::ChunkHandle(const ChunkHandle& rhs) {
    *this = std::move(ChunkHandle::acquire_existing(rhs));
}

hvox::ChunkHandle& hvox::ChunkHandle::operator=(const ChunkHandle& rhs) {
    *this = std::move(ChunkHandle::acquire_existing(rhs));

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

hvox::ChunkHandle hvox::ChunkHandle::acquire_existing(const ChunkHandle& handle) {
    ChunkHandle new_handle;

    new_handle.m_chunk = handle.m_chunk;

    if (new_handle == nullptr)
        return new_handle;

    new_handle->ref_count++;

    if (new_handle->alive_state.load(std::memory_order_acquire) == ChunkAliveState::DEAD) {
        new_handle->ref_count.store(0, std::memory_order_release);
        new_handle.m_chunk = nullptr;
    }

    return new_handle;
}

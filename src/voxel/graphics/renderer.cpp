#include "stdafx.h"

#include "voxel/block.hpp"
#include "voxel/chunk.h"

#include "voxel/graphics/renderer.h"

hg::MeshHandles hvox::ChunkRenderer::block_mesh_handles = {};

hvox::ChunkRenderer::ChunkRenderer() :
    handle_chunk_mesh_change(Subscriber<>{ [&](Sender sender) {
        hmem::WeakHandle<Chunk> handle = sender.get_handle<Chunk>();

        auto chunk = handle.lock();
        // If chunk is nullptr, then there's no point
        // handling the mesh change as we will have
        // an unload event for this chunk.
        if (chunk == nullptr) return;

        m_chunk_dirty_queue.enqueue({ handle, chunk->id() });
    } }),
    handle_chunk_unload(Subscriber<>{ [&](Sender sender) {
        hmem::WeakHandle<Chunk> handle = sender.get_handle<Chunk>();

        auto chunk = handle.lock();
        // If chunk is nullptr, then we have
        // a major problem. on_unload MUST
        // complete before chunk is released.
        assert(chunk != nullptr);

        m_chunk_removal_queue.enqueue({ handle, chunk->id() });
    } }),
    m_page_size(0) { /* Empty. */
}

void hvox::ChunkRenderer::init(ui32 page_size, ui32 max_unused_pages) {
    if (block_mesh_handles.vao == 0) {
        hg::upload_mesh(BLOCK_MESH, block_mesh_handles, hg::MeshDataVolatility::STATIC);

        glEnableVertexArrayAttrib(block_mesh_handles.vao, 3);
        glVertexArrayAttribFormat(block_mesh_handles.vao, 3, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(block_mesh_handles.vao, 3, 1);

        glEnableVertexArrayAttrib(block_mesh_handles.vao, 4);
        glVertexArrayAttribFormat(
            block_mesh_handles.vao, 4, 3, GL_FLOAT, GL_FALSE, sizeof(f32v3)
        );
        glVertexArrayAttribBinding(block_mesh_handles.vao, 4, 1);

        glVertexArrayBindingDivisor(block_mesh_handles.vao, 1, 1);
    }

    m_page_size        = page_size;
    m_max_unused_pages = max_unused_pages;

    // Create pages up to max unused pages so we don't
    // do as much allocation later.
    create_pages(m_max_unused_pages);
}

void hvox::ChunkRenderer::dispose() {
    m_page_size = 0;

    for (auto& chunk_page : m_chunk_pages) {
        glDeleteBuffers(1, &chunk_page->vbo);
        delete chunk_page;
    }
    ChunkRenderPages().swap(m_chunk_pages);

    // TODO(Matthew): Should do refcounting like in outline renderer for block mesh
    // handles.
    //                  Less important here as we probably never don't have some chunk
    //                  renderer in existence, and it's all very cheap.
}

void hvox::ChunkRenderer::set_page_size(ui32 page_size) {
    m_page_size = page_size;
}

void hvox::ChunkRenderer::update(FrameTime) {
    // TODO(Matthew): Do we really want to de-dirty pages every update?
    //                Perhaps track how long since we last did and try
    //                to spread this out. But likewise do we want to
    //                render a chunk to a page on mesh completion, or
    //                wait a bit in-case of a player moving fast?

    process_pages();
}

void hvox::ChunkRenderer::draw(FrameTime) {
    glBindVertexArray(block_mesh_handles.vao);
    for (auto& chunk_page : m_chunk_pages) {
        if (chunk_page->voxel_count == 0) continue;

        glVertexArrayVertexBuffer(
            block_mesh_handles.vao, 1, chunk_page->vbo, 0, sizeof(ChunkInstanceData)
        );

        glDrawArraysInstanced(
            GL_TRIANGLES, 0, BLOCK_VERTEX_COUNT, chunk_page->voxel_count
        );
    }
}

void hvox::ChunkRenderer::add_chunk(hmem::WeakHandle<Chunk> handle) {
    auto chunk = handle.lock();

    if (chunk == nullptr) return;

    chunk->on_mesh_change += &handle_chunk_mesh_change;
    chunk->on_unload      += &handle_chunk_unload;

    m_all_paged_chunks[chunk->id()] = handle;
    m_chunk_metadata[chunk->id()]   = PagedChunkMetadata{};
}

hvox::ChunkRenderPage* hvox::ChunkRenderer::create_pages(ui32 count) {
    m_chunk_pages.reserve(m_chunk_pages.size() + count);

    /*
     * For now we create GPU buffers at the size of the page, we were
     * using a page-grow approach but this seems costly for little
     * memory saving up-front, and none for a long-running session.
     */

    /**************************\
     * Append first new page. *
    \**************************/

    m_chunk_pages.emplace_back(new ChunkRenderPage{});

    ChunkRenderPage* first_new_page = m_chunk_pages.back();

    glCreateBuffers(1, &first_new_page->vbo);
    glNamedBufferData(
        first_new_page->vbo,
        block_page_size() * sizeof(ChunkInstanceData),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    first_new_page->chunks.reserve(m_page_size);

    /********************************\
     * Append subsequent new pages. *
    \********************************/
    for (ui32 i = 1; i < count; ++i) {
        m_chunk_pages.emplace_back(new ChunkRenderPage{});

        ChunkRenderPage* new_page = m_chunk_pages.back();

        glCreateBuffers(1, &new_page->vbo);
        glNamedBufferData(
            new_page->vbo,
            block_page_size() * sizeof(ChunkInstanceData),
            nullptr,
            GL_DYNAMIC_DRAW
        );

        new_page->chunks.reserve(m_page_size);
    }

    // Return pointer to the first page created.
    return first_new_page;
}

void hvox::ChunkRenderer::put_chunk_in_page(
    ChunkID chunk_id, ui32 instance_count, ui32 first_page_idx
) {
    PagedChunkMetadata& metadata = m_chunk_metadata[chunk_id];

    ui32             page_idx = first_page_idx;
    ChunkRenderPage* page     = nullptr;
    for (; page_idx < m_chunk_pages.size(); ++page_idx) {
        if (m_chunk_pages[page_idx]->voxel_count + instance_count <= block_page_size())
        {
            page = m_chunk_pages[page_idx];
            break;
        }
    }

    if (page == nullptr) {
        page = create_pages(1);
    }

    ui32 chunk_idx = static_cast<ui32>(page->chunks.size());
    page->chunks.emplace_back(chunk_id);

    if (page->first_dirtied_chunk_idx >= page->chunks.size()) {
        page->first_dirtied_chunk_idx = static_cast<ui32>(page->chunks.size() - 1);
    }

    page->voxel_count += instance_count;

    page->dirty = true;

    metadata.chunk_idx = chunk_idx;
    metadata.page_idx  = page_idx;
    metadata.paged     = true;
}

void hvox::ChunkRenderer::process_pages() {
    HandleAndID handle_and_id;

    /*****************\
     * Remove Chunks *
    \*****************/

    while (m_chunk_removal_queue.try_dequeue(handle_and_id)) {
        auto it = m_chunk_metadata.find(handle_and_id.id);

        assert(it != m_chunk_metadata.end());

        PagedChunkMetadata metadata = it->second;

        m_chunk_metadata.erase(it);

        if (!metadata.paged) continue;

        ChunkRenderPage& page = *m_chunk_pages[metadata.page_idx];

        // Only do full clean up if the chunk we're removing is not
        // at the end of the page already.
        if (metadata.chunk_idx != page.chunks.size() - 1) {
            std::swap(page.chunks.back(), page.chunks[metadata.chunk_idx]);
            page.chunks.pop_back();

            // TODO(Matthew): This updates chunk indexes, but feels like it
            //                isn't the best approach. Perhaps we can do
            //                better?
            for (ui32 chunk_idx = metadata.chunk_idx; chunk_idx < page.chunks.size();
                 ++chunk_idx)
            {
                m_chunk_metadata[page.chunks[chunk_idx]].chunk_idx = chunk_idx;
            }

            if (page.first_dirtied_chunk_idx > metadata.chunk_idx) {
                page.first_dirtied_chunk_idx = metadata.chunk_idx;
            }

            // Make sure page is reprocessed.
            page.dirty = true;
        } else {
            page.chunks.pop_back();

            // If only dirty chunk was the one we just removed, and
            // given we just popped the final chunk in the page, we
            // can mark as clean.
            if (page.first_dirtied_chunk_idx == metadata.chunk_idx) {
                page.first_dirtied_chunk_idx = std::numeric_limits<ui32>::max();

                page.dirty = false;
            }
        }

        // Deduct removed voxels from page voxel count, allowing
        // a new chunk to join if extra space permits.
        page.voxel_count -= metadata.on_gpu_voxel_count;
    }

    /*****************\
     * Update Chunks *
    \*****************/

    while (m_chunk_dirty_queue.try_dequeue(handle_and_id)) {
        auto it = m_chunk_metadata.find(handle_and_id.id);

        if (it == m_chunk_metadata.end()) continue;

        PagedChunkMetadata& metadata = it->second;
        metadata.dirty               = true;

        auto chunk = handle_and_id.handle.lock();

        if (metadata.paged) {
            ChunkRenderPage& page = *m_chunk_pages[metadata.page_idx];

            if (page.first_dirtied_chunk_idx > metadata.chunk_idx) {
                page.first_dirtied_chunk_idx = metadata.chunk_idx;
            }

            page.dirty = true;
        } else if (chunk != nullptr) {
            std::shared_lock<std::shared_mutex> instance_lock;
            const auto& instance = chunk->instance.get(instance_lock);

            put_chunk_in_page(chunk->id(), instance.count, 0);
        }
    }

    /*****************\
     * Process Pages *
    \*****************/

    // TODO(Matthew): Could change this to simply two VBOs per page, and swap back
    //                and forth between them. That way we don't keep creating
    //                new buffer objects (we'd still want to deallocate buffers).

    // We will swap all prior-existing pages' VBOs with these.
    // For undirtied pages this will not actually change their VBO.
    std::vector<GLuint> new_vbos(m_chunk_pages.size(), 0);

    for (ui32 page_idx = 0; page_idx < m_chunk_pages.size(); ++page_idx) {
        ChunkRenderPage& page = *m_chunk_pages[page_idx];
        if (!page.dirty) {
            new_vbos[page_idx] = page.vbo;
            continue;
        }

        assert(page.first_dirtied_chunk_idx < page.chunks.size());

        // Create a new on-GPU buffer to populate.
        glCreateBuffers(1, &new_vbos[page_idx]);
        glNamedBufferData(
            new_vbos[page_idx],
            block_page_size() * sizeof(ChunkInstanceData),
            nullptr,
            GL_DYNAMIC_DRAW
        );

        ui32 voxels_instanced = 0;
        for (ui32 chunk_idx = 0; chunk_idx < page.first_dirtied_chunk_idx; ++chunk_idx)
        {
            voxels_instanced
                += m_chunk_metadata[page.chunks[chunk_idx]].on_gpu_voxel_count;
        }

        // Copy unchanged original data into new buffer.
        glCopyNamedBufferSubData(
            page.vbo,
            new_vbos[page_idx],
            0,
            0,
            voxels_instanced * sizeof(ChunkInstanceData)
        );

        for (ui32 chunk_idx = page.first_dirtied_chunk_idx;
             chunk_idx < page.chunks.size();)
        {
            ChunkID id = page.chunks[chunk_idx];

            auto it = m_chunk_metadata.find(id);
            assert(it != m_chunk_metadata.end());
            PagedChunkMetadata& metadata = it->second;

            auto chunk = m_all_paged_chunks[id].lock();

            // "Dirty" chunk that has ceased to exist, can't
            // update it so drop it. Don't need to do anything
            // special as it will have been added to the removal
            // queue.
            if (chunk == nullptr) continue;

            // Is chunk actually dirty (i.e. does a new set of instance
            // data exist to upload to the GPU), or can we copy its previous
            // data from wherever that lies on the GPU?
            if (metadata.dirty) {
                std::shared_lock<std::shared_mutex> instance_lock;
                const auto& instance = chunk->instance.get(instance_lock);

                assert(instance.count <= block_page_size());

                // Check chunk still fits in this page, if not, remove it and place
                // it in a page that might still have space for it (else creating a
                // new page for it).
                if (voxels_instanced + instance.count > block_page_size()) {
                    // Swap non-fitting chunk with last chunk in page, pop it,
                    // and update the metadata for the chunk that we swapped in.
                    std::swap(page.chunks[chunk_idx], page.chunks.back());
                    page.chunks.pop_back();
                    metadata.chunk_idx = chunk_idx;

                    // TODO(Matthew): Does this lead to too much memory use? Perhaps
                    //                do a shuffle phase to fit all chunks and then
                    //                do the instance data processing logic.
                    put_chunk_in_page(chunk->id(), instance.count, page_idx + 1);

                    // We don't want to do any more processing of this chunk just yet.
                    // chunk_idx now indexes to what was previously the last chunk in
                    // this page.
                    continue;
                }

                glNamedBufferSubData(
                    new_vbos[page_idx],
                    voxels_instanced * sizeof(ChunkInstanceData),
                    instance.count * sizeof(ChunkInstanceData),
                    reinterpret_cast<void*>(instance.data)
                );
                metadata.on_gpu_offset      = voxels_instanced;
                metadata.on_gpu_page_idx    = page_idx;
                metadata.on_gpu_voxel_count = instance.count;

                voxels_instanced += instance.count;

                metadata.dirty = false;
            } else {
                glCopyNamedBufferSubData(
                    m_chunk_pages[metadata.on_gpu_page_idx]->vbo,
                    new_vbos[page_idx],
                    metadata.on_gpu_offset * sizeof(ChunkInstanceData),
                    voxels_instanced * sizeof(ChunkInstanceData),
                    metadata.on_gpu_voxel_count * sizeof(ChunkInstanceData)
                );
                metadata.on_gpu_offset   = voxels_instanced;
                metadata.on_gpu_page_idx = page_idx;

                voxels_instanced += metadata.on_gpu_voxel_count;
            }

            chunk->instance.free_buffer();

            ++chunk_idx;
        }

        page.voxel_count             = voxels_instanced;
        page.dirty                   = false;
        page.first_dirtied_chunk_idx = std::numeric_limits<ui32>::max();
    }

    for (ui32 page_idx = 0; page_idx < new_vbos.size(); ++page_idx) {
        if (new_vbos[page_idx] != m_chunk_pages[page_idx]->vbo) {
            glDeleteBuffers(1, &m_chunk_pages[page_idx]->vbo);
            m_chunk_pages[page_idx]->vbo = new_vbos[page_idx];
        }
    }
}

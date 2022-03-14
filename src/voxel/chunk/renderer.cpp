#include "stdafx.h"

#include "voxel/block.hpp"
#include "voxel/chunk.h"

#include "voxel/chunk/renderer.h"

hg::MeshHandles hvox::ChunkRenderer::block_mesh_handles = {};

hvox::ChunkRenderer::ChunkRenderer() :
    handle_chunk_mesh_change(Subscriber<>{
        [&](Sender sender) {
            const Chunk* chunk = reinterpret_cast<const Chunk*>(sender);

            m_chunk_dirty_queue.enqueue(chunk);
        }
    }),
    handle_chunk_unload(Subscriber<>{
        [&](Sender sender) {
            const Chunk* chunk = reinterpret_cast<const Chunk*>(sender);

            m_chunk_removal_queue.enqueue(chunk);
        }
    }),
    m_page_size(0)
{ /* Empty. */ }

void hvox::ChunkRenderer::init(ui32 page_size, ui32 max_unused_pages) {
    if (block_mesh_handles.vao == 0) {
        hg::upload_mesh(BLOCK_MESH, block_mesh_handles, hg::MeshDataVolatility::STATIC);

        glEnableVertexArrayAttrib(block_mesh_handles.vao,  3);
        glVertexArrayAttribFormat(block_mesh_handles.vao,  3, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(block_mesh_handles.vao, 3, 1);

        glEnableVertexArrayAttrib(block_mesh_handles.vao,  4);
        glVertexArrayAttribFormat(block_mesh_handles.vao,  4, 3, GL_FLOAT, GL_FALSE, sizeof(f32v3));
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
        glDeleteBuffers(1, &chunk_page.vbo);
    }
    ChunkRenderPages().swap(m_chunk_pages);
}

void hvox::ChunkRenderer::set_page_size(ui32 page_size) {
    m_page_size = page_size;
}

void hvox::ChunkRenderer::update(TimeData) {
    // TODO(Matthew): Do we really want to de-dirty pages every update?
    //                Perhaps track how long since we last did and try
    //                to spread this out. But likewise do we want to
    //                render a chunk to a page on mesh completion, or
    //                wait a bit in-case of a player moving fast?

    process_pages();
}

void hvox::ChunkRenderer::draw(TimeData) {
    glBindVertexArray(block_mesh_handles.vao);
    for (auto& chunk_page : m_chunk_pages) {
        if (chunk_page.voxel_count == 0) continue;

        glVertexArrayVertexBuffer(block_mesh_handles.vao, 1, chunk_page.vbo, 0, sizeof(ChunkInstanceData));

        glDrawArraysInstanced(GL_TRIANGLES, 0, BLOCK_VERTEX_COUNT, chunk_page.voxel_count);
    }
}

void hvox::ChunkRenderer::add_chunk(Chunk* chunk) {
    chunk->on_mesh_change += &handle_chunk_mesh_change;
    chunk->on_unload      += &handle_chunk_unload;

    m_chunk_metadata[chunk] = PagedChunkMetadata{};
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

    m_chunk_pages.emplace_back(ChunkRenderPage{});

    ChunkRenderPage* first_new_page = &m_chunk_pages.back();

    glCreateBuffers(1, &first_new_page->vbo);
    glNamedBufferData(first_new_page->vbo, block_page_size() * sizeof(ChunkInstanceData), nullptr, GL_DYNAMIC_DRAW);

    first_new_page->chunks.reserve(m_page_size);

    /********************************\
     * Append subsequent new pages. *
    \********************************/
    for (ui32 i = 0; i < count; ++i) {
        m_chunk_pages.emplace_back(ChunkRenderPage{});

        ChunkRenderPage* new_page = &m_chunk_pages.back();

        glCreateBuffers(1, &new_page->vbo);
        glNamedBufferData(new_page->vbo, block_page_size() * sizeof(ChunkInstanceData), nullptr, GL_DYNAMIC_DRAW);

        new_page->chunks.reserve(m_page_size);
    }

    // Return pointer to the first page created.
    return first_new_page;
}

void hvox::ChunkRenderer::put_chunk_in_page(const Chunk* chunk, ui32 first_page_idx) {
    ChunkRenderPage* page = nullptr;
    for (ui32 i = first_page_idx; i < m_chunk_pages.size(); ++i) {
        if (m_chunk_pages[i].voxel_count + chunk->instance.count <= block_page_size()) {
            page = &m_chunk_pages[i];
            break;
        }
    }

    if (page == nullptr) {
        page = create_pages(1);
    }

    page->chunks.emplace_back(chunk);

    if (page->first_dirtied_chunk_idx >= page->chunks.size()) {
        page->first_dirtied_chunk_idx  = page->chunks.size() - 1;
    }

    page->voxel_count += chunk->instance.count;

    page->dirty = true;
}

void hvox::ChunkRenderer::process_pages() {
    // TODO(Matthew): Lock on chunk instance data? Perhaps
    //                we can use double buffering to avoid that?

    const Chunk* chunk;

    /*****************\
     * Remove Chunks *
    \*****************/

    while (m_chunk_removal_queue.try_dequeue(chunk)) {
        PagedChunkMetadata metadata = m_chunk_metadata[chunk];

        if (!metadata.paged) continue;

        ChunkRenderPage& page = m_chunk_pages[metadata.page_idx];

        std::swap(page.chunks.back(), page.chunks[metadata.chunk_idx]);
        page.chunks.pop_back();

        if (page.first_dirtied_chunk_idx > metadata.chunk_idx) {
            page.first_dirtied_chunk_idx = metadata.chunk_idx;
        }

        page.dirty = true;
    }

    /*****************\
     * Update Chunks *
    \*****************/

    while (m_chunk_dirty_queue.try_dequeue(chunk)) {
        PagedChunkMetadata metadata = m_chunk_metadata[chunk];

        if (metadata.paged) {
            ChunkRenderPage& page = m_chunk_pages[metadata.page_idx];

            if (page.first_dirtied_chunk_idx > metadata.chunk_idx) {
                page.first_dirtied_chunk_idx = metadata.chunk_idx;
            }

            page.dirty = true;
        } else {
            put_chunk_in_page(chunk, 0);
        }
    }

    /*****************\
     * Process Pages *
    \*****************/

    for (ui32 page_idx = 0; page_idx < m_chunk_pages.size(); ++page_idx) {
        ChunkRenderPage& page = m_chunk_pages[page_idx];
        if (!page.dirty) continue;

        assert(page.first_dirtied_chunk_idx < page.chunks.size());

        ui32 voxels_instanced = 0;
        for (ui32 chunk_idx = 0; chunk_idx < page.first_dirtied_chunk_idx; ++chunk_idx) {
            voxels_instanced += page.chunks[chunk_idx]->instance.count;
        }

        for (ui32 chunk_idx = page.first_dirtied_chunk_idx; chunk_idx < page.chunks.size();) {
            chunk = page.chunks[chunk_idx];
            auto data = chunk->instance;

            // Check chunk still fits in this page, if not, remove it and place
            // it in a page that might still have space for it (else creating a
            // new page for it).
            if (page.voxel_count + data.count > block_page_size()) {
                std::swap(page.chunks[chunk_idx], page.chunks.back());
                page.chunks.pop_back();
                // TODO(Matthew): Does this lead to too much memory use? Perhaps
                //                search all pages for space, but do a double
                //                pass of this page processing logic?
                put_chunk_in_page(chunk, page_idx + 1);
                // We don't want to do any more processing of this chunk just yet.
                // chunk_idx now indexes to what was previously the last chunk in
                // this page.
                continue;
            }

            glNamedBufferSubData(
                page.vbo,
                voxels_instanced * sizeof(ChunkInstanceData),
                data.count       * sizeof(ChunkInstanceData),
                reinterpret_cast<void*>(data.data)
            );

            voxels_instanced += data.count;

            ++chunk_idx;
        }

        page.voxel_count = voxels_instanced;
        page.dirty = false;
        page.first_dirtied_chunk_idx = std::numeric_limits<ui32>::max();
    }
}

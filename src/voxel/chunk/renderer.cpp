#include "stdafx.h"

#include "voxel/block.hpp"
#include "voxel/chunk.h"

#include "voxel/chunk/renderer.h"

hg::MeshHandles hvox::ChunkRenderer::block_mesh_handles = {};

hvox::ChunkRenderer::ChunkRenderer() :
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

    
}

void hvox::ChunkRenderer::draw(TimeData) {
    glBindVertexArray(block_mesh_handles.vao);
    for (auto& chunk_page : m_chunk_pages) {
        glVertexArrayVertexBuffer(block_mesh_handles.vao, 1, chunk_page.vbo, 0, sizeof(ChunkInstanceData));

        glDrawArraysInstanced(GL_TRIANGLES, 0, BLOCK_VERTEX_COUNT, chunk_page.voxel_count);
    }
}

void hvox::ChunkRenderer::render_chunk(Chunk* chunk) {
    ChunkRenderPage* target_page = nullptr;
    for (auto& chunk_page : m_chunk_pages) {
        if (chunk_page.voxel_count + chunk->instance.count <= block_page_size()) {
            target_page = &chunk_page;
            break;
        }
    }

    if (target_page == nullptr) {
        target_page = create_pages(1);
    }

    target_page->chunks[target_page->chunk_count] = PagedChunk{
        .chunk = chunk
    };

    if (target_page->first_dirtied_chunk_idx > target_page->chunks.size()) {
        target_page->first_dirtied_chunk_idx = target_page->chunks.size();
    }

    target_page->voxel_count += chunk->instance.count;

    target_page->dirty = true;
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
    glNamedBufferData(first_new_page->vbo, m_page_size * sizeof(ChunkInstanceData), nullptr, GL_STATIC_DRAW);

    first_new_page->chunks.reserve(m_page_size);

    /********************************\
     * Append subsequent new pages. *
    \********************************/
    for (ui32 i = 0; i < count; ++i) {
        m_chunk_pages.emplace_back(ChunkRenderPage{});

        ChunkRenderPage* new_page = &m_chunk_pages.back();

        glCreateBuffers(1, &new_page->vbo);
        glNamedBufferData(new_page->vbo, m_page_size * sizeof(ChunkInstanceData), nullptr, GL_STATIC_DRAW);

        new_page->chunks.reserve(m_page_size);
    }

    // Return pointer to the first page created.
    return first_new_page;
}

void hvox::ChunkRenderer::process_page(ui32 page_id) {
    ChunkRenderPage& page = m_chunk_pages[page_id];

    ui32 start_from_chunk = page.first_dirtied_chunk_idx;

    // if (page.gpu_alloc_size < page.voxel_count) {
    //     page.gpu_alloc_size = page.voxel_count;

    //     glNamedBufferData(page.vbo, page.voxel_count * sizeof(ChunkInstanceData), nullptr, GL_STATIC_DRAW);

    //     start_from_chunk = 0;
    // }

    ui32 cursor = 0;
    for (ui32 i = 0; i < start_from_chunk; ++i) {
        cursor += page.chunks[i].chunk->instance.count;
    }

    for (ui32 i = start_from_chunk; i < page.chunks.size(); ++i) {
        auto data = page.chunks[i].chunk->instance;

        glNamedBufferSubData(
            page.vbo,
            cursor * sizeof(ChunkInstanceData),
            data.count * sizeof(ChunkInstanceData),
            reinterpret_cast<void*>(data.data)
        );

        cursor += data.count;
    }

    page.first_dirtied_chunk_idx = std::numeric_limits<ui32>::max();
}

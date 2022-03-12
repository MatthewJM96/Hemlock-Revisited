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
        delete[] chunk_page.instance_data;

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
        // TODO(Matthew): bind correct VBO to the VAO.

        glDrawArraysInstanced(GL_TRIANGLES, 0, BLOCK_VERTEX_COUNT, chunk_page.voxel_count);
    }
}

void hvox::ChunkRenderer::render_chunk(Chunk* chunk) {
    ChunkRenderPage* target_page = nullptr;
    for (auto& chunk_page : m_chunk_pages) {
        if (chunk_page.voxel_count + chunk->instance.count <= m_page_size) {
            target_page = &chunk_page;
            break;
        }
    }

    if (target_page == nullptr) {
        target_page = create_pages(1);
    }

    std::memcpy(
        &target_page->instance_data[target_page->voxel_count],
        chunk->instance.data,
        chunk->instance.count * sizeof(ChunkInstanceData)
    );
    target_page->voxel_count += chunk->instance.count;

    target_page->dirty = true;
}

hvox::ChunkRenderPage* hvox::ChunkRenderer::create_pages(ui32 count) {
    m_chunk_pages.reserve(m_chunk_pages.size() + count);

    /**************************\
     * Append first new page. *
    \**************************/

    m_chunk_pages.emplace_back(ChunkRenderPage{});

    ChunkRenderPage* first_new_page = &m_chunk_pages.back();

    first_new_page->instance_data = new ChunkInstanceData[m_page_size];

    /********************************\
     * Append subsequent new pages. *
    \********************************/
    for (ui32 i = 0; i < count; ++i) {
        m_chunk_pages.emplace_back(ChunkRenderPage{});

        ChunkRenderPage* new_page = &m_chunk_pages.back();

        new_page->instance_data = new ChunkInstanceData[m_page_size];
    }

    // Return pointer to the first page created.
    return first_new_page;
}

#include "stdafx.h"

#include "voxel/graphics/outline_renderer/navmesh.h"

#include "maths/powers.hpp"

hvox::NavmeshOutlineRenderer::NavmeshOutlineRenderer() :
    handle_chunk_navmesh_change(Delegate<void(Sender)>{ [&](Sender) {

    } }),
    handle_chunk_unload(Delegate<void(Sender)>{ [&](Sender) {

    } }),
    m_navmesh_mesh_handles{},
    m_next_outline_id(1),
    m_last_outline_count(0) {
    // Empty.
}

void hvox::NavmeshOutlineRenderer::init() {
    hg::upload_mesh(
        hg::Point_2D_32_MeshData{ nullptr, 0 },
        m_navmesh_mesh_handles,
        hg::MeshDataVolatility::STATIC
    );
}

void hvox::NavmeshOutlineRenderer::dispose() {
    BlockOutlines().swap(m_block_outlines);
    BlockOutlineRefs().swap(m_block_outline_refs);

    ui32 prev_ref_count = ref_count.fetch_sub(1);
    if (prev_ref_count == 1) {
        hg::dispose_mesh(m_navmesh_mesh_handles);

        m_navmesh_mesh_handles = {};
    }
}

void hvox::NavmeshOutlineRenderer::draw(FrameTime) {
    size_t last_size = hmaths::next_power_2(m_last_outline_count);
    size_t this_size = hmaths::next_power_2(m_block_outlines.size());

    // If VBO buffer needs to be increased to fit in all outlines, then do so.
    // TODO(Matthew): shrink this also?
    if (last_size < this_size) {
        glNamedBufferData(
            m_instance_vbo, this_size * sizeof(OutlineData), nullptr, GL_DYNAMIC_DRAW
        );
    }

    // TODO(Matthew): We are sending this data every frame?! Might be fine
    //                for debug purposes, but we gotta be sure this is
    //                all people want to use this for.
    glNamedBufferSubData(
        m_instance_vbo,
        0,
        m_block_outlines.size() * sizeof(OutlineData),
        reinterpret_cast<void*>(m_block_outlines.data())
    );

    glBindVertexArray(m_navmesh_mesh_handles.vao);

    glVertexArrayVertexBuffer(
        m_navmesh_mesh_handles.vao, 1, m_instance_vbo, 0, sizeof(OutlineData)
    );

    glDrawArraysInstanced(
        GL_LINES, 0, BLOCK_OUTLINE_VERTEX_COUNT, m_block_outlines.size()
    );
}

size_t hvox::NavmeshOutlineRenderer::add_outline(OutlineData&& outline) {
    auto [_, success]
        = m_block_outline_refs.try_emplace(m_next_outline_id, m_block_outlines.size());

    if (!success) return 0;

    m_block_outlines.emplace_back(std::forward<OutlineData>(outline));

    return m_next_outline_id++;
}

bool hvox::NavmeshOutlineRenderer::modify_outline(
    size_t outline_id, OutlineData&& outline
) {
    try {
        size_t idx = m_block_outline_refs.at(outline_id);

        m_block_outlines[idx] = std::forward<OutlineData>(outline);
    } catch (std::out_of_range& e) {
        return false;
    }

    return true;
}

bool hvox::NavmeshOutlineRenderer::remove_outline(size_t outline_id) {
    auto it = m_block_outline_refs.find(outline_id);
    if (it != m_block_outline_refs.end()) {
        m_block_outlines[it->second] = std::move(m_block_outlines.back());

        auto back_it = std::find_if(
            m_block_outline_refs.begin(),
            m_block_outline_refs.end(),
            [&](auto el) { return el.second == m_block_outlines.size() - 1; }
        );

        back_it->second = it->second;

        m_block_outlines.pop_back();

        m_block_outline_refs.erase(it);
    } else {
        return false;
    }

    return true;
}

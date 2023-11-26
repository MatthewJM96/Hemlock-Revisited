#include "stdafx.h"

#include "voxel/graphics/outline_renderer/block.h"

#include "maths/powers.hpp"

hg::MeshHandles   hvox::BlockOutlineRenderer::block_mesh_handles = {};
std::atomic<ui32> hvox::BlockOutlineRenderer::ref_count          = 0;

hvox::BlockOutlineRenderer::BlockOutlineRenderer() :
    m_next_outline_id(1), m_last_outline_count(0) {
    // Empty.
}

void hvox::BlockOutlineRenderer::init() {
#if !defined(HEMLOCK_OS_MAC)
    glCreateBuffers(1, &m_instance_vbo);

    ui32 prev_ref_count = ref_count.fetch_add(1);
    if (prev_ref_count == 0) {
        hg::upload_mesh(
            BLOCK_OUTLINE_MESH, block_mesh_handles, hg::MeshDataVolatility::STATIC
        );

        glEnableVertexArrayAttrib(block_mesh_handles.vao, 1);
        glVertexArrayAttribFormat(block_mesh_handles.vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(block_mesh_handles.vao, 1, 1);

        glEnableVertexArrayAttrib(block_mesh_handles.vao, 2);
        glVertexArrayAttribFormat(
            block_mesh_handles.vao, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(f32v3)
        );
        glVertexArrayAttribBinding(block_mesh_handles.vao, 2, 1);

        glVertexArrayBindingDivisor(block_mesh_handles.vao, 1, 1);
    }
#else   // !defined(HEMLOCK_OS_MAC)
    glGenBuffers(1, &m_instance_vbo);

    ui32 prev_ref_count = ref_count.fetch_add(1);
    if (prev_ref_count == 0) {
        hg::upload_mesh(
            BLOCK_OUTLINE_MESH, block_mesh_handles, hg::MeshDataVolatility::STATIC
        );

        glBindVertexArray(block_mesh_handles.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(OutlineData),
            static_cast<void*>(offsetof(OutlineData, position))
        );
        glEnableVertexArrayAttrib(1);

        glVertexAttribPointer(
            2,
            4,
            GL_UNSIGNED_BYTE,
            GL_TRUE,
            sizeof(OutlineData),
            static_cast<void*>(offsetof(OutlineData, position))
        );
        glEnableVertexArrayAttrib(2);

        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
#endif  // !defined(HEMLOCK_OS_MAC)
}

void hvox::BlockOutlineRenderer::dispose() {
    BlockOutlines().swap(m_block_outlines);
    BlockOutlineRefs().swap(m_block_outline_refs);

    ui32 prev_ref_count = ref_count.fetch_sub(1);
    if (prev_ref_count == 1) {
        hg::dispose_mesh(block_mesh_handles);

        block_mesh_handles = {};
    }
}

void hvox::BlockOutlineRenderer::draw(FrameTime) {
    size_t last_size = hmaths::next_power_2(m_last_outline_count);
    size_t this_size = hmaths::next_power_2(m_block_outlines.size());

    glBindVertexArray(block_mesh_handles.vao);

#if !defined(HEMLOCK_OS_MAC)
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

    glVertexArrayVertexBuffer(
        block_mesh_handles.vao, 1, m_instance_vbo, 0, sizeof(OutlineData)
    );
#else   // !defined(HEMLOCK_OS_MAC)
    glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

    // If VBO buffer needs to be increased to fit in all outlines, then do so.
    // TODO(Matthew): shrink this also?
    if (last_size < this_size) {
        glBufferData(
            GL_ARRAY_BUFFER, this_size * sizeof(OutlineData), nullptr, GL_DYNAMIC_DRAW
        );
    }

    // TODO(Matthew): We are sending this data every frame?! Might be fine
    //                for debug purposes, but we gotta be sure this is
    //                all people want to use this for.
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        m_block_outlines.size() * sizeof(OutlineData),
        reinterpret_cast<void*>(m_block_outlines.data())
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif  // !defined(HEMLOCK_OS_MAC)

    glDrawArraysInstanced(
        GL_LINES,
        0,
        BLOCK_OUTLINE_VERTEX_COUNT,
        static_cast<GLsizei>(m_block_outlines.size())
    );

    glBindVertexArray(0);
}

size_t hvox::BlockOutlineRenderer::add_outline(OutlineData&& outline) {
    auto [_, success]
        = m_block_outline_refs.try_emplace(m_next_outline_id, m_block_outlines.size());

    if (!success) return 0;

    m_block_outlines.emplace_back(std::forward<OutlineData>(outline));

    return m_next_outline_id++;
}

bool hvox::BlockOutlineRenderer::modify_outline(
    size_t outline_id, OutlineData&& outline
) {
    try {
        size_t idx = m_block_outline_refs.at(outline_id);

        m_block_outlines[idx] = std::forward<OutlineData>(outline);
    } catch (std::out_of_range&) {
        return false;
    }

    return true;
}

bool hvox::BlockOutlineRenderer::remove_outline(size_t outline_id) {
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

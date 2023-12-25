#include "stdafx.h"

#include "voxel/graphics/outline_renderer/voxel.h"

#include "maths/powers.hpp"

hg::MeshHandles   hvox::VoxelOutlineRenderer::voxel_mesh_handles = {};
std::atomic<ui32> hvox::VoxelOutlineRenderer::ref_count          = 0;

hvox::VoxelOutlineRenderer::VoxelOutlineRenderer() :
    m_next_outline_id(1), m_last_outline_count(0) {
    // Empty.
}

void hvox::VoxelOutlineRenderer::init() {
#if !defined(HEMLOCK_OS_MAC)
    glCreateBuffers(1, &m_instance_vbo);

    ui32 prev_ref_count = ref_count.fetch_add(1);
    if (prev_ref_count == 0) {
        hg::upload_mesh(
            VOXEL_OUTLINE_MESH, voxel_mesh_handles, hg::MeshDataVolatility::STATIC
        );

        glEnableVertexArrayAttrib(voxel_mesh_handles.vao, 1);
        glVertexArrayAttribFormat(voxel_mesh_handles.vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(voxel_mesh_handles.vao, 1, 1);

        glEnableVertexArrayAttrib(voxel_mesh_handles.vao, 2);
        glVertexArrayAttribFormat(
            voxel_mesh_handles.vao, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(f32v3)
        );
        glVertexArrayAttribBinding(voxel_mesh_handles.vao, 2, 1);

        glVertexArrayBindingDivisor(voxel_mesh_handles.vao, 1, 1);
    }
#else   // !defined(HEMLOCK_OS_MAC)
    glGenBuffers(1, &m_instance_vbo);

    ui32 prev_ref_count = ref_count.fetch_add(1);
    if (prev_ref_count == 0) {
        hg::upload_mesh(
            VOXEL_OUTLINE_MESH, voxel_mesh_handles, hg::MeshDataVolatility::STATIC
        );

        glBindVertexArray(voxel_mesh_handles.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(OutlineData),
            reinterpret_cast<void*>(offsetof(OutlineData, position))
        );
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(
            2,
            4,
            GL_UNSIGNED_BYTE,
            GL_TRUE,
            sizeof(OutlineData),
            reinterpret_cast<void*>(offsetof(OutlineData, position))
        );
        glEnableVertexAttribArray(2);

        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
#endif  // !defined(HEMLOCK_OS_MAC)
}

void hvox::VoxelOutlineRenderer::dispose() {
    VoxelOutlines().swap(m_voxel_outlines);
    VoxelOutlineRefs().swap(m_voxel_outline_refs);

    ui32 prev_ref_count = ref_count.fetch_sub(1);
    if (prev_ref_count == 1) {
        hg::dispose_mesh(voxel_mesh_handles);

        voxel_mesh_handles = {};
    }
}

void hvox::VoxelOutlineRenderer::draw(FrameTime) {
    size_t last_size = hmaths::next_power_2(m_last_outline_count);
    size_t this_size = hmaths::next_power_2(m_voxel_outlines.size());

    glBindVertexArray(voxel_mesh_handles.vao);

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
        m_voxel_outlines.size() * sizeof(OutlineData),
        reinterpret_cast<void*>(m_voxel_outlines.data())
    );

    glVertexArrayVertexBuffer(
        voxel_mesh_handles.vao, 1, m_instance_vbo, 0, sizeof(OutlineData)
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
        m_voxel_outlines.size() * sizeof(OutlineData),
        reinterpret_cast<void*>(m_voxel_outlines.data())
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif  // !defined(HEMLOCK_OS_MAC)

    glDrawArraysInstanced(
        GL_LINES,
        0,
        VOXEL_OUTLINE_VERTEX_COUNT,
        static_cast<GLsizei>(m_voxel_outlines.size())
    );

    glBindVertexArray(0);
}

size_t hvox::VoxelOutlineRenderer::add_outline(OutlineData&& outline) {
    auto [_, success]
        = m_voxel_outline_refs.try_emplace(m_next_outline_id, m_voxel_outlines.size());

    if (!success) return 0;

    m_voxel_outlines.emplace_back(std::forward<OutlineData>(outline));

    return m_next_outline_id++;
}

bool hvox::VoxelOutlineRenderer::modify_outline(
    size_t outline_id, OutlineData&& outline
) {
    try {
        size_t idx = m_voxel_outline_refs.at(outline_id);

        m_voxel_outlines[idx] = std::forward<OutlineData>(outline);
    } catch (std::out_of_range&) {
        return false;
    }

    return true;
}

bool hvox::VoxelOutlineRenderer::remove_outline(size_t outline_id) {
    auto it = m_voxel_outline_refs.find(outline_id);
    if (it != m_voxel_outline_refs.end()) {
        m_voxel_outlines[it->second] = std::move(m_voxel_outlines.back());

        auto back_it = std::find_if(
            m_voxel_outline_refs.begin(),
            m_voxel_outline_refs.end(),
            [&](auto el) { return el.second == m_voxel_outlines.size() - 1; }
        );

        back_it->second = it->second;

        m_voxel_outlines.pop_back();

        m_voxel_outline_refs.erase(it);
    } else {
        return false;
    }

    return true;
}

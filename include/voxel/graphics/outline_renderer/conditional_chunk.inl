#include "voxel/chunk/grid.h"

template <hvox::OutlinePredicate Pred>
hg::MeshHandles hvox::ConditionalChunkOutlineRenderer<Pred>::chunk_mesh_handles = {};
template <hvox::OutlinePredicate Pred>
std::atomic<ui32> hvox::ConditionalChunkOutlineRenderer<Pred>::ref_count = 0;

template <hvox::OutlinePredicate Pred>
hvox::ConditionalChunkOutlineRenderer<Pred>::ConditionalChunkOutlineRenderer() :
    handle_render_distance_change(Delegate<void(Sender, RenderDistanceChangeEvent)>{
        [&](Sender, RenderDistanceChangeEvent ev) {
            // NOTE(Matthew): Could reduce new calls but really render distance of a
            // chunk grid
            //                shouldn't change all that often, and this particular
            //                renderer is intended for debug purposes.
            // NOTE(Matthew): Assumption here that render distance change events are
            // only ever
            //                called from the main thread with regards to the chunk
            //                grid this renderer is acting on. This thread also being
            //                the only thread to ever call draw for a call to which a
            //                race condition would otherwise occur with how this is
            //                implemented.
            if (m_chunk_outline_conditions) delete[] m_chunk_outline_conditions;

            m_chunk_outline_conditions
                = new OutlineData[ev.after.chunks_in_render_distance];
#if !defined(HEMLOCK_OS_MAC)
            glNamedBufferData(
                m_instance_vbo,
                ev.after.chunks_in_render_distance * sizeof(OutlineData),
                nullptr,
                GL_DYNAMIC_DRAW
            );
#else   // !defined(HEMLOCK_OS_MAC)
            glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
            glBufferData(
                GL_ARRAY_BUFFER,
                ev.after.chunks_in_render_distance * sizeof(OutlineData),
                nullptr,
                GL_DYNAMIC_DRAW
            );
            glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif  // !defined(HEMLOCK_OS_MAC)
        } }) {
    // Empty.
}

template <hvox::OutlinePredicate Pred>
void hvox::ConditionalChunkOutlineRenderer<Pred>::init(
    Pred predicate, hmem::Handle<ChunkGrid> chunk_grid
) {
    m_predicate  = predicate;
    m_chunk_grid = chunk_grid;

    m_chunk_outline_conditions
        = new OutlineData[chunk_grid->chunks_in_render_distance()];

#if !defined(HEMLOCK_OS_MAC)
    glCreateBuffers(1, &m_instance_vbo);
    glNamedBufferData(
        m_instance_vbo,
        chunk_grid->chunks_in_render_distance() * sizeof(OutlineData),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    ui32 prev_ref_count = ref_count.fetch_add(1);
    if (prev_ref_count == 0) {
        hg::upload_mesh(
            CHUNK_OUTLINE_MESH, chunk_mesh_handles, hg::MeshDataVolatility::STATIC
        );

        glEnableVertexArrayAttrib(chunk_mesh_handles.vao, 1);
        glVertexArrayAttribFormat(chunk_mesh_handles.vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(chunk_mesh_handles.vao, 1, 1);

        glEnableVertexArrayAttrib(chunk_mesh_handles.vao, 2);
        glVertexArrayAttribFormat(
            chunk_mesh_handles.vao, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(f32v3)
        );
        glVertexArrayAttribBinding(chunk_mesh_handles.vao, 2, 1);

        glVertexArrayBindingDivisor(chunk_mesh_handles.vao, 1, 1);
    }
#else   // !defined(HEMLOCK_OS_MAC)
    glGenBuffers(1, &m_instance_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        chunk_grid->chunks_in_render_distance() * sizeof(OutlineData),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    ui32 prev_ref_count = ref_count.fetch_add(1);
    if (prev_ref_count == 0) {
        hg::upload_mesh(
            CHUNK_OUTLINE_MESH, chunk_mesh_handles, hg::MeshDataVolatility::STATIC
        );

        glBindVertexArray(chunk_mesh_handles.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(OutlineData),
            static_cast<void*>(offsetof(OutlineData, position))
        );
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(
            2,
            4,
            GL_UNSIGNED_BYTE,
            GL_TRUE,
            sizeof(OutlineData),
            static_cast<void*>(offsetof(OutlineData, colour))
        );
        glEnableVertexAttribArray(2);

        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);

        glBindVertexArray(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif  // !defined(HEMLOCK_OS_MAC)
}

template <hvox::OutlinePredicate Pred>
void hvox::ConditionalChunkOutlineRenderer<Pred>::dispose() {
    m_chunk_grid.reset();

    delete[] m_chunk_outline_conditions;

    ui32 prev_ref_count = ref_count.fetch_sub(1);
    if (prev_ref_count == 1) {
        hg::dispose_mesh(chunk_mesh_handles);

        chunk_mesh_handles = {};
    }
}

template <hvox::OutlinePredicate Pred>
void hvox::ConditionalChunkOutlineRenderer<Pred>::draw(FrameTime) {
    m_chunk_outline_condition_count = 0;

    for (auto& [id, chunk] : m_chunk_grid->chunks()) {
        auto [should_draw, colour] = m_predicate(chunk);

        if (should_draw) {
            m_chunk_outline_conditions[m_chunk_outline_condition_count++]
                = OutlineData{ f32v3(block_world_position(chunk->position, 0)),
                               colour };
        }
    }

    glBindVertexArray(chunk_mesh_handles.vao);
    // TODO(Matthew): We are sending this data every frame?! Might be fine
    //                for debug purposes, but we gotta be sure this is
    //                all people want to use this for.
#if !defined(HEMLOCK_OS_MAC)
    glNamedBufferSubData(
        m_instance_vbo,
        0,
        m_chunk_outline_condition_count * sizeof(OutlineData),
        reinterpret_cast<void*>(m_chunk_outline_conditions)
    );

    glVertexArrayVertexBuffer(
        chunk_mesh_handles.vao, 1, m_instance_vbo, 0, sizeof(OutlineData)
    );
#else   // !defined(HEMLOCK_OS_MAC)
    glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        m_chunk_outline_condition_count * sizeof(OutlineData),
        reinterpret_cast<void*>(m_chunk_outline_conditions)
    );
#endif  // !defined(HEMLOCK_OS_MAC)

    glDrawArraysInstanced(
        GL_LINES, 0, CHUNK_OUTLINE_VERTEX_COUNT, m_chunk_outline_condition_count
    );
}

template <hvox::ChunkOutlinePredicate Pred>
hg::MeshHandles   hvox::ConditionalChunkOutlineRenderer<Pred>::chunk_mesh_handles = {};
template <hvox::ChunkOutlinePredicate Pred>
std::atomic<ui32> hvox::ConditionalChunkOutlineRenderer<Pred>::ref_count          = 0;

template <hvox::ChunkOutlinePredicate Pred>
hvox::ConditionalChunkOutlineRenderer<Pred>::ConditionalChunkOutlineRenderer() :
    handle_render_distance_change(Delegate<void(Sender, RenderDistanceChangeEvent)>{
        [&](Sender, RenderDistanceChangeEvent ev) {
            // NOTE(Matthew): Could reduce new calls but really render distance of a chunk grid
            //                shouldn't change all that often, and this particular renderer is
            //                intended for debug purposes.
            // NOTE(Matthew): Assumption here that render distance change events are only ever
            //                called from the main thread with regards to the chunk grid this renderer
            //                is acting on. This thread also being the only thread to ever call draw
            //                for a call to which a race condition would otherwise occur with how this
            //                is implemented.
            m_chunk_outline_conditions = new ChunkOutlineCondition[ev.after.chunks_in_render_distance];

            glNamedBufferData(m_instance_vbo, ev.after.chunks_in_render_distance * sizeof(ChunkOutlineCondition), nullptr, GL_DYNAMIC_DRAW);
        }
    })
{
    // Empty.
}

template <hvox::ChunkOutlinePredicate Pred>
void hvox::ConditionalChunkOutlineRenderer<Pred>::init(Pred predicate, hmem::Handle<ChunkGrid> chunk_grid) {
    m_predicate  = predicate;
    m_chunk_grid = chunk_grid;

    m_chunk_outline_conditions = new ChunkOutlineCondition[chunk_grid->chunks_in_render_distance()];

    glCreateBuffers(1, &m_instance_vbo);
    glNamedBufferData(m_instance_vbo, chunk_grid->chunks_in_render_distance() * sizeof(ChunkOutlineCondition), nullptr, GL_DYNAMIC_DRAW);

    ui32 prev_ref_count = ref_count.fetch_add(1);
    if (prev_ref_count == 0) {
        hg::upload_mesh(CHUNK_OUTLINE_MESH, chunk_mesh_handles, hg::MeshDataVolatility::STATIC);

        glEnableVertexArrayAttrib(chunk_mesh_handles.vao,  1);
        glVertexArrayAttribFormat(chunk_mesh_handles.vao,  1, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(chunk_mesh_handles.vao, 1, 1);

        glEnableVertexArrayAttrib(chunk_mesh_handles.vao,  2);
        glVertexArrayAttribFormat(chunk_mesh_handles.vao,  2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(f32v3));
        glVertexArrayAttribBinding(chunk_mesh_handles.vao, 2, 1);

        glVertexArrayBindingDivisor(chunk_mesh_handles.vao, 1, 1);
    }
}

template <hvox::ChunkOutlinePredicate Pred>
void hvox::ConditionalChunkOutlineRenderer<Pred>::dispose() {
    m_chunk_grid.reset();

    delete[] m_chunk_outline_conditions;

    ui32 prev_ref_count = ref_count.fetch_sub(1);
    if (prev_ref_count == 1) {
        hg::dispose_mesh(chunk_mesh_handles);

        chunk_mesh_handles = {};
    }
}

template <hvox::ChunkOutlinePredicate Pred>
void hvox::ConditionalChunkOutlineRenderer<Pred>::draw(FrameTime) {
    m_chunk_outline_condition_count = 0;

    for (auto& [id, chunk] : m_chunk_grid->chunks()) {
        auto [should_draw, colour] = m_predicate(chunk);

        if (should_draw) {
            m_chunk_outline_conditions[m_chunk_outline_condition_count++]
                = ChunkOutlineCondition{ f32v3(block_world_position(chunk->position, 0)), colour };
        }
    }

    // TODO(Matthew): We are sending this data every frame?! Might be fine
    //                for debug purposes, but we gotta be sure this is
    //                all people want to use this for.
    glNamedBufferSubData(m_instance_vbo, 0, m_chunk_outline_condition_count * sizeof(ChunkOutlineCondition), reinterpret_cast<void*>(m_chunk_outline_conditions));

    glBindVertexArray(chunk_mesh_handles.vao);

    glVertexArrayVertexBuffer(chunk_mesh_handles.vao, 1, m_instance_vbo, 0, sizeof(ChunkOutlineCondition));

    glDrawArraysInstanced(GL_LINES, 0, CHUNK_OUTLINE_VERTEX_COUNT, m_chunk_outline_condition_count);
}

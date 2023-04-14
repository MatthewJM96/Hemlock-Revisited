#include "stdafx.h"

#include "voxel/chunk.h"
#include "voxel/graphics/outline_renderer/navmesh.h"

#include "maths/powers.hpp"

hvox::NavmeshOutlineRenderer::NavmeshOutlineRenderer() :
    handle_chunk_navmesh_change(Delegate<void(Sender)>{ [&](Sender sender) {
        auto chunk = sender.get_handle<Chunk>().lock();

        m_navmesh_outlines[chunk->position.id].is_dirty.store(true);
    } }),
    handle_chunk_unload(Delegate<void(Sender)>{ [&](Sender sender) {
        auto chunk = sender.get_handle<Chunk>().lock();

        m_navmesh_outlines[chunk->position.id].is_dead.store(true);
    } }),
    m_navmesh_outline_vao(0) {
    // Empty.
}

void hvox::NavmeshOutlineRenderer::init() {
    hg::MeshHandles tmp = { .vao = 0, .vbo = static_cast<GLuint>(-1) };
    hg::upload_mesh(
        hg::RGBA_Point_3D_32_MeshData{ nullptr, 0 }, tmp, hg::MeshDataVolatility::STATIC
    );
    m_navmesh_outline_vao = tmp.vao;
}

void hvox::NavmeshOutlineRenderer::dispose() {
    for (auto& [chunk_id, navmesh] : m_navmesh_outlines) {
        hg::dispose_mesh(hg::MeshHandles{ 0, navmesh.mesh_handles.vbo });
    }
    hg::dispose_mesh(hg::MeshHandles{ m_navmesh_outline_vao, 0 });

    m_navmesh_outline_vao = 0;

    NavmeshOutlinesPerChunk().swap(m_navmesh_outlines);
}

void hvox::NavmeshOutlineRenderer::update(FrameTime) {
    for (auto& [chunk_id, navmesh] : m_navmesh_outlines) {
        if (navmesh.is_dirty.load()) {
            if (__calculate_outlines(navmesh)) navmesh.is_updated = true;
            navmesh.is_dirty.store(false);
        }
    }
}

void hvox::NavmeshOutlineRenderer::draw(FrameTime) {
    for (auto& [chunk_id, navmesh] : m_navmesh_outlines) {
        if (navmesh.is_updated && !navmesh.is_dead.load()) {
            size_t last_size = hmaths::next_power_2(navmesh.last_size);
            size_t this_size = hmaths::next_power_2(navmesh.outlines.size());

            if (last_size < this_size) {
                glNamedBufferData(
                    navmesh.mesh_handles.vbo,
                    this_size * sizeof(NavmeshOutlineData),
                    nullptr,
                    GL_DYNAMIC_DRAW
                );

                glNamedBufferSubData(
                    navmesh.mesh_handles.vbo,
                    0,
                    navmesh.outlines.size() * sizeof(NavmeshOutlineData),
                    reinterpret_cast<void*>(navmesh.outlines.data())
                );

                navmesh.last_size = this_size;
            }

            navmesh.is_updated = false;
        }

        if (navmesh.outlines.size() == 0) continue;

        glBindVertexArray(navmesh.mesh_handles.vao);

        glVertexArrayVertexBuffer(
            navmesh.mesh_handles.vao,
            0,
            navmesh.mesh_handles.vbo,
            0,
            sizeof(NavmeshOutlineDatum)
        );

        glDrawArrays(GL_LINES, 0, navmesh.outlines.size() * 2);
    }
}

void hvox::NavmeshOutlineRenderer::register_chunk(hmem::Handle<Chunk> chunk) {
    NavmeshOutlines tmp = {};
    tmp.chunk           = chunk;
    auto [it, success]  = m_navmesh_outlines.try_emplace(chunk->position.id, tmp);

    if (success) {
        it->second.mesh_handles = { m_navmesh_outline_vao, 0 };
        hg::upload_mesh(
            hg::RGBA_Point_3D_32_MeshData{ nullptr, 0 }, it->second.mesh_handles
        );

        chunk->on_navmesh_change += &handle_chunk_navmesh_change;
        chunk->on_unload         += &handle_chunk_unload;
    }
}

bool hvox::NavmeshOutlineRenderer::__calculate_outlines(NavmeshOutlines& navmesh) {
    hmem::Handle<Chunk> chunk = navmesh.chunk.lock();

    if (chunk == nullptr) {
        navmesh.is_dead.store(true);
        return false;
    }

    auto lock = std::shared_lock(chunk->navmesh_mutex);

    if (chunk->navmeshing.load() != ChunkState::COMPLETE) return false;

    std::vector<NavmeshOutlineData> tmp_outline_buffer;
    tmp_outline_buffer.reserve(navmesh.outlines.size());

    for (auto vertex :
         boost::make_iterator_range(boost::vertices(chunk->navmesh.graph)))
    {
        ai::ChunkNavmeshNode node = chunk->navmesh.vertex_coord_map[vertex];
        BlockWorldPosition   vert_pos
            = block_world_position(node.chunk_pos, node.block_pos);

        for (auto edge :
             boost::make_iterator_range(boost::out_edges(vertex, chunk->navmesh.graph)))
        {
            auto target_vertex = boost::target(edge, chunk->navmesh.graph);

            ai::ChunkNavmeshNode target_node
                = chunk->navmesh.vertex_coord_map[target_vertex];
            BlockWorldPosition target_vert_pos
                = block_world_position(target_node.chunk_pos, target_node.block_pos);

            tmp_outline_buffer.emplace_back(NavmeshOutlineData{
                NavmeshOutlineDatum{static_cast<f32v3>(vert_pos)
+ f32v3{ 0.5f, 1.6f, 0.5f },
                                    { 255, 0, 0, 255 }},
                NavmeshOutlineDatum{static_cast<f32v3>(target_vert_pos)
+ f32v3{ 0.5f, 1.6f, 0.5f },
                                    { 255, 0, 0, 255 }}
            });
        }
    }

    navmesh.outlines.swap(tmp_outline_buffer);

    return true;
}

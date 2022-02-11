#include "stdafx.h"

#include "graphics/mesh.h"

#ifdef HEMLOCK_USING_OPENGL
template <bool indexed, ui32 vertex_size, ui32 precision>
static bool create_vao_for_basic_mesh(auto mesh_data, hg::MeshDataVolatility volatility, IN OUT GLuint* vao, IN OUT GLuint* vbo, IN OUT GLuint* ebo) {
    if (!mesh_data.vertices) return 0;

    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * mesh_data.vertex_count, mesh_data.vertices, static_cast<GLenum>(volatility));

    if constexpr (indexed) {
        glGenBuffers(1, ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, precision * mesh_data.index_count, mesh_data.indices, static_cast<GLenum>(volatility));
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, vertex_size, reinterpret_cast<GLvoid*>(3 * precision));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertex_size, reinterpret_cast<GLvoid*>(7 * precision));
    glEnableVertexAttribArray(2);

    if constexpr (indexed) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}
#endif // HEMLOCK_USING_OPENGL


bool hg::create_vao(
    const MeshData2D_32& mesh_data,
            MeshHandles& handles,
      MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return create_vao_for_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 4>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::create_vao(
    const MeshData2D_64& mesh_data,
            MeshHandles& handles,
      MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return create_vao_for_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 8>(mesh_data, volatility, &handles.vao, &handles.vao, nullptr);
}

bool hg::create_vao(
    const MeshData3D_32& mesh_data,
            MeshHandles& handles,
      MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return create_vao_for_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 4>(mesh_data, volatility, &handles.vao, &handles.vao, nullptr);
}

bool hg::create_vao(
    const MeshData3D_64& mesh_data,
            MeshHandles& handles,
      MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return create_vao_for_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 8>(mesh_data, volatility, &handles.vao, &handles.vao, nullptr);
}

bool hg::create_vao(
    const IndexedMeshData2D_32& mesh_data,
            IndexedMeshHandles& handles,
             MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return create_vao_for_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 4>(mesh_data, volatility, &handles.vao, &handles.vao, &handles.ebo);
}

bool hg::create_vao(
    const IndexedMeshData2D_64& mesh_data,
            IndexedMeshHandles& handles,
             MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return create_vao_for_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 8>(mesh_data, volatility, &handles.vao, &handles.vao, &handles.ebo);
}

bool hg::create_vao(
    const IndexedMeshData3D_32& mesh_data,
            IndexedMeshHandles& handles,
             MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return create_vao_for_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 4>(mesh_data, volatility, &handles.vao, &handles.vao, &handles.ebo);
}

bool hg::create_vao(
    const IndexedMeshData3D_64& mesh_data,
            IndexedMeshHandles& handles,
             MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return create_vao_for_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 8>(mesh_data, volatility, &handles.vao, &handles.vao, &handles.ebo);
}

#include "stdafx.h"

#include "graphics/mesh.h"

#include "graphics/mesh/common_vertices_def.hpp"

#if defined(HEMLOCK_USING_OPENGL)
#  include "graphics/mesh/gl.hpp"
#endif  // defined(HEMLOCK_USING_OPENGL)

H_DEF_ENUM_WITH_SERIALISATION(hemlock::graphics, MeshDataVolatility)

GEN_MESH_UPLOADER_DEFS(Point_2D_32, MAKE_POS2D(32))

GEN_MESH_UPLOADER_DEFS(Point_3D_32, MAKE_POS3D(32))

GEN_MESH_UPLOADER_DEFS(Point_2D_64, MAKE_POS2D(64))

GEN_MESH_UPLOADER_DEFS(Point_3D_64, MAKE_POS3D(64))

GEN_MESH_UPLOADER_DEFS(Colourless_2D_32, MAKE_POS2D(32), MAKE_UV(32))

GEN_MESH_UPLOADER_DEFS(Colourless_3D_32, MAKE_POS3D(32), MAKE_UV(32))

GEN_MESH_UPLOADER_DEFS(Colourless_2D_64, MAKE_POS2D(64), MAKE_UV(64))

GEN_MESH_UPLOADER_DEFS(Colourless_3D_64, MAKE_POS3D(64), MAKE_UV(64))

GEN_MESH_UPLOADER_DEFS(RGB_2D_32, MAKE_POS2D(32), MAKE_UV(32), MAKE_RGB(32))

GEN_MESH_UPLOADER_DEFS(RGB_3D_32, MAKE_POS3D(32), MAKE_UV(32), MAKE_RGB(32))

GEN_MESH_UPLOADER_DEFS(RGB_2D_64, MAKE_POS2D(64), MAKE_UV(64), MAKE_RGB(64))

GEN_MESH_UPLOADER_DEFS(RGB_3D_64, MAKE_POS3D(64), MAKE_UV(64), MAKE_RGB(64))

GEN_MESH_UPLOADER_DEFS(RGBA_2D_32, MAKE_POS2D(32), MAKE_UV(32), MAKE_RGBA(32))

GEN_MESH_UPLOADER_DEFS(RGBA_3D_32, MAKE_POS3D(32), MAKE_UV(32), MAKE_RGBA(32))

GEN_MESH_UPLOADER_DEFS(RGBA_2D_64, MAKE_POS2D(64), MAKE_UV(64), MAKE_RGBA(64))

GEN_MESH_UPLOADER_DEFS(RGBA_3D_64, MAKE_POS3D(64), MAKE_UV(64), MAKE_RGBA(64))

GEN_MESH_UPLOADER_DEFS(
    Colourless_2D_32_Normal, MAKE_POS2D(32), MAKE_UV(32), MAKE_NORMAL2D(32)
)

GEN_MESH_UPLOADER_DEFS(
    Colourless_3D_32_Normal, MAKE_POS3D(32), MAKE_UV(32), MAKE_NORMAL3D(32)
)

GEN_MESH_UPLOADER_DEFS(
    Colourless_2D_64_Normal, MAKE_POS2D(64), MAKE_UV(64), MAKE_NORMAL2D(64)
)

GEN_MESH_UPLOADER_DEFS(
    Colourless_3D_64_Normal, MAKE_POS3D(64), MAKE_UV(64), MAKE_NORMAL3D(64)
)

GEN_MESH_UPLOADER_DEFS(
    RGB_2D_32_Normal, MAKE_POS2D(32), MAKE_UV(32), MAKE_RGB(32), MAKE_NORMAL2D(32)
)

GEN_MESH_UPLOADER_DEFS(
    RGB_3D_32_Normal, MAKE_POS3D(32), MAKE_UV(32), MAKE_RGB(32), MAKE_NORMAL3D(32)
)

GEN_MESH_UPLOADER_DEFS(
    RGB_2D_64_Normal, MAKE_POS2D(64), MAKE_UV(64), MAKE_RGB(64), MAKE_NORMAL2D(64)
)

GEN_MESH_UPLOADER_DEFS(
    RGB_3D_64_Normal, MAKE_POS3D(64), MAKE_UV(64), MAKE_RGB(64), MAKE_NORMAL3D(64)
)

GEN_MESH_UPLOADER_DEFS(
    RGBA_2D_32_Normal, MAKE_POS2D(32), MAKE_UV(32), MAKE_RGBA(32), MAKE_NORMAL2D(32)
)

GEN_MESH_UPLOADER_DEFS(
    RGBA_3D_32_Normal, MAKE_POS3D(32), MAKE_UV(32), MAKE_RGBA(32), MAKE_NORMAL3D(32)
)

GEN_MESH_UPLOADER_DEFS(
    RGBA_2D_64_Normal, MAKE_POS2D(64), MAKE_UV(64), MAKE_RGBA(64), MAKE_NORMAL2D(64)
)

GEN_MESH_UPLOADER_DEFS(
    RGBA_3D_64_Normal, MAKE_POS3D(64), MAKE_UV(64), MAKE_RGBA(64), MAKE_NORMAL3D(64)
)

#include "graphics/mesh/common_vertices_undef.hpp"

#if defined(HEMLOCK_USING_OPENGL)
template <bool indexed>
static void __dispose_mesh(GLuint vao, GLuint vbo, GLuint ibo) {
    assert(vao != 0);
    assert(vbo != 0);
    if constexpr (indexed) assert(ibo != 0);

    glDeleteBuffers(1, &vbo);
    if constexpr (indexed) glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &vao);
}
#endif  // defined(HEMLOCK_USING_OPENGL)

void hg::dispose_mesh(const MeshHandles& handles) {
    __dispose_mesh<false>(handles.vao, handles.vbo, 0);
}

void hg::dispose_mesh(const IndexedMeshHandles& handles) {
    __dispose_mesh<true>(handles.vao, handles.vbo, handles.ibo);
}

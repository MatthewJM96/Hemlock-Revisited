#include "stdafx.h"

#include "graphics/mesh.h"

#define MAKE_POS2D(PRECISION)   (Vertex_Pos2D_##PRECISION,  position,   POSITION,   2,  PRECISION,  0)
#define MAKE_POS3D(PRECISION)   (Vertex_Pos3D_##PRECISION,  position,   POSITION,   3,  PRECISION,  0)
#define MAKE_UV(PRECISION)      (Vertex_UV_##PRECISION,     uv,         UV,         2,  PRECISION,  0)
#define MAKE_RGB(PRECISION)     (Vertex_RGB_##PRECISION,    rgb,        RGB,        3,  PRECISION,  1)
#define MAKE_RGBA(PRECISION)    (Vertex_RGBA_##PRECISION,   rgba,       RGBA,       4,  PRECISION,  1)

GEN_MESH_UPLOADER_DEFS(
    Colourless_2D_32,
    MAKE_POS2D(32),
    MAKE_UV(32)
)

GEN_MESH_UPLOADER_DEFS(
    Colourless_3D_32,
    MAKE_POS3D(32),
    MAKE_UV(32)
)

GEN_MESH_UPLOADER_DEFS(
    Colourless_2D_64,
    MAKE_POS2D(64),
    MAKE_UV(64)
)

GEN_MESH_UPLOADER_DEFS(
    Colourless_3D_64,
    MAKE_POS3D(64),
    MAKE_UV(64)
)

GEN_MESH_UPLOADER_DEFS(
    RGB_2D_32,
    MAKE_POS2D(32),
    MAKE_UV(32),
    MAKE_RGB(32)
)

GEN_MESH_UPLOADER_DEFS(
    RGB_3D_32,
    MAKE_POS3D(32),
    MAKE_UV(32),
    MAKE_RGB(32)
)

GEN_MESH_UPLOADER_DEFS(
    RGB_2D_64,
    MAKE_POS2D(64),
    MAKE_UV(64),
    MAKE_RGB(64)
)

GEN_MESH_UPLOADER_DEFS(
    RGB_3D_64,
    MAKE_POS3D(64),
    MAKE_UV(64),
    MAKE_RGB(64)
)

GEN_MESH_UPLOADER_DEFS(
    RGBA_2D_32,
    MAKE_POS2D(32),
    MAKE_UV(32),
    MAKE_RGBA(32)
)

GEN_MESH_UPLOADER_DEFS(
    RGBA_3D_32,
    MAKE_POS3D(32),
    MAKE_UV(32),
    MAKE_RGBA(32)
)

GEN_MESH_UPLOADER_DEFS(
    RGBA_2D_64,
    MAKE_POS2D(64),
    MAKE_UV(64),
    MAKE_RGBA(64)
)

GEN_MESH_UPLOADER_DEFS(
    RGBA_3D_64,
    MAKE_POS3D(64),
    MAKE_UV(64),
    MAKE_RGBA(64)
)

#undef MAKE_RGBA
#undef MAKE_RGB
#undef MAKE_UV
#undef MAKE_POS3D
#undef MAKE_POS2D

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
#endif // defined(HEMLOCK_USING_OPENGL)

void hg::dispose_mesh(const MeshHandles& handles) {
    __dispose_mesh<false>(handles.vao, handles.vbo, 0);
}

void hg::dispose_mesh(const IndexedMeshHandles& handles) {
    __dispose_mesh<true>(handles.vao, handles.vbo, handles.ibo);
}

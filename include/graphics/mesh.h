#ifndef __hemlock_graphics_mesh_h
#define __hemlock_graphics_mesh_h

#include "io/serialisation.hpp"

#if defined(HEMLOCK_USING_OPENGL)
/**
 * @brief Enumerates the mesh volatility states.
 */
H_DECL_VENUM_WITH_SERIALISATION(
    hemlock::graphics,
    MeshDataVolatility,
    ui32,
    (STATIC, GL_STATIC_DRAW),
    (DYNAMIC, GL_DYNAMIC_DRAW),
    (STREAM, GL_STREAM_DRAW)
)
#endif  // defined(HEMLOCK_USING_OPENGL)

namespace hemlock {
    namespace graphics {
        struct MeshHandles {
#if defined(HEMLOCK_USING_OPENGL)
            GLuint vao, vbo;
#endif  // defined(HEMLOCK_USING_OPENGL)
        };

        struct IndexedMeshHandles : public MeshHandles {
#if defined(HEMLOCK_USING_OPENGL)
            GLuint ibo;
#endif  // defined(HEMLOCK_USING_OPENGL)
        };

        // IMPROVEMENT(Matthew):
        // While we have a macro-based way of handling decls & defs
        // associated with vertices, meshes and associated upload
        // functions we would like to be able to use templating
        // for this. Doing a templated form is to my knowledge
        // not nicely possible as composing the struct would not
        // be a verifiable process. Perhaps we don't care about that
        // or otherwise there is some trick to doing it anyway.
        //   Certainly reflection should make it possible in
        //   a sensible way.

        using Vertex_Pos2D_32    = f32v2;
        using Vertex_Pos2D_64    = f64v2;
        using Vertex_Pos3D_32    = f32v3;
        using Vertex_Pos3D_64    = f32v3;
        using Vertex_Normal2D_32 = f32v2;
        using Vertex_Normal2D_64 = f64v2;
        using Vertex_Normal3D_32 = f32v3;
        using Vertex_Normal3D_64 = f32v3;
        using Vertex_RGB_32      = f32v3;
        using Vertex_RGB_64      = f64v3;
        using Vertex_RGBA_64     = f64v4;
        using Vertex_RGBA_32     = f32v4;
        using Vertex_UV_32       = f32v2;
        using Vertex_UV_64       = f64v2;

        struct IndexData {
            ui32* indices;
            ui32  index_count;
        };

#define VERTEX_FIELD_TYPE(                                                             \
    FIELD_TYPE, FIELD_NAME, ENUM_NAME, ELEM_COUNT, PRECISION, NORMALISED               \
)                                                                                      \
  FIELD_TYPE
#define VERTEX_FIELD_NAME(                                                             \
    FIELD_TYPE, FIELD_NAME, ENUM_NAME, ELEM_COUNT, PRECISION, NORMALISED               \
)                                                                                      \
  FIELD_NAME
#define VERTEX_ENUM_NAME(                                                              \
    FIELD_TYPE, FIELD_NAME, ENUM_NAME, ELEM_COUNT, PRECISION, NORMALISED               \
)                                                                                      \
  ENUM_NAME
#define VERTEX_ELEM_COUNT(                                                             \
    FIELD_TYPE, FIELD_NAME, ENUM_NAME, ELEM_COUNT, PRECISION, NORMALISED               \
)                                                                                      \
  ELEM_COUNT
#define VERTEX_PRECISION(                                                              \
    FIELD_TYPE, FIELD_NAME, ENUM_NAME, ELEM_COUNT, PRECISION, NORMALISED               \
)                                                                                      \
  PRECISION
#define VERTEX_NORMALISED(                                                             \
    FIELD_TYPE, FIELD_NAME, ENUM_NAME, ELEM_COUNT, PRECISION, NORMALISED               \
)                                                                                      \
  NORMALISED

#define GEN_MESH_ATTRIB_ENUM_FIELD(VERTEX_INFO, ID)                                    \
  VERTEX_ENUM_NAME VERTEX_INFO = ID_TO_INT(#ID)

#define GEN_MESH_ATTRIB_ENUM_DEF(PREFIX, ...)                                          \
  enum class PREFIX##_MeshAttribID{                                                    \
      MAP_WITH_ID(GEN_MESH_ATTRIB_ENUM_FIELD, COMMA, __VA_ARGS__), SENTINEL            \
  };

#define GEN_VERTEX_STRUCT_FIELD(VERTEX_INFO)                                           \
  VERTEX_FIELD_TYPE VERTEX_INFO VERTEX_FIELD_NAME VERTEX_INFO;

#define GEN_VERTEX_STRUCT_DEF(PREFIX, ...)                                             \
  struct PREFIX##_Vertex {                                                             \
    MAP(GEN_VERTEX_STRUCT_FIELD, EMPTY, __VA_ARGS__)                                   \
  };

#define GEN_UNINDEXED_MESH_DATA_STRUCT(PREFIX)                                         \
  struct PREFIX##_MeshData {                                                           \
    PREFIX##_Vertex* vertices;                                                         \
    ui32             vertex_count;                                                     \
  };

#define GEN_INDEXED_MESH_DATA_STRUCT(PREFIX)                                           \
  struct PREFIX##_IndexedMeshData :                                                    \
      public PREFIX##_MeshData,                                                        \
      public IndexData { /* Empty. */                                                  \
  };

#define GEN_MESH_DATA_STRUCT_DEFS(PREFIX)                                              \
  GEN_UNINDEXED_MESH_DATA_STRUCT(PREFIX)                                               \
  GEN_INDEXED_MESH_DATA_STRUCT(PREFIX)

#define GEN_UNINDEXED_MESH_UPLOADER_DECL(PREFIX)                                       \
  bool upload_mesh(                                                                    \
      const PREFIX##_MeshData& mesh_data,                                              \
      OUT MeshHandles&         handles,                                                \
      MeshDataVolatility       volatility = MeshDataVolatility::DYNAMIC                \
  );

#define GEN_INDEXED_MESH_UPLOADER_DECL(PREFIX)                                         \
  bool upload_mesh(                                                                    \
      const PREFIX##_IndexedMeshData& mesh_data,                                       \
      OUT IndexedMeshHandles&         handles,                                         \
      MeshDataVolatility              volatility = MeshDataVolatility::DYNAMIC         \
  );

#define GEN_MESH_UPLOADER_DEFS(PREFIX, ...)                                            \
  GEN_MESH_UPLOADER_DEF(PREFIX, 0, __VA_ARGS__)                                        \
  GEN_MESH_UPLOADER_DEF(PREFIX, 1, __VA_ARGS__)

#define GEN_MESH_UPLOADER_DECLS(PREFIX)                                                \
  GEN_UNINDEXED_MESH_UPLOADER_DECL(PREFIX)                                             \
  GEN_INDEXED_MESH_UPLOADER_DECL(PREFIX)

#define GEN_MESH_CASE_STRUCT_DEFS(PREFIX, ...)                                         \
  GEN_MESH_ATTRIB_ENUM_DEF(PREFIX, __VA_ARGS__)                                        \
  GEN_VERTEX_STRUCT_DEF(PREFIX, __VA_ARGS__)                                           \
  GEN_MESH_DATA_STRUCT_DEFS(PREFIX)

#define GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(PREFIX, ...)                          \
  GEN_MESH_CASE_STRUCT_DEFS(PREFIX, __VA_ARGS__)                                       \
  GEN_MESH_UPLOADER_DECLS(PREFIX)

#include "graphics/mesh/common_vertices_def.hpp"

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(Point_2D_32, MAKE_POS2D(32))

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(Point_3D_32, MAKE_POS3D(32))

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(Point_2D_64, MAKE_POS2D(64))

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(Point_3D_64, MAKE_POS3D(64))

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            Colourless_2D_32, MAKE_POS2D(32), MAKE_UV(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            Colourless_3D_32, MAKE_POS3D(32), MAKE_UV(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            Colourless_2D_64, MAKE_POS2D(64), MAKE_UV(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            Colourless_3D_64, MAKE_POS3D(64), MAKE_UV(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGB_2D_32, MAKE_POS2D(32), MAKE_UV(32), MAKE_RGB(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGB_3D_32, MAKE_POS3D(32), MAKE_UV(32), MAKE_RGB(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGB_2D_64, MAKE_POS2D(64), MAKE_UV(64), MAKE_RGB(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGB_3D_64, MAKE_POS3D(64), MAKE_UV(64), MAKE_RGB(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGBA_2D_32, MAKE_POS2D(32), MAKE_UV(32), MAKE_RGBA(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGBA_3D_32, MAKE_POS3D(32), MAKE_UV(32), MAKE_RGBA(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGBA_2D_64, MAKE_POS2D(64), MAKE_UV(64), MAKE_RGBA(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGBA_3D_64, MAKE_POS3D(64), MAKE_UV(64), MAKE_RGBA(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            Colourless_2D_32_Normal, MAKE_POS2D(32), MAKE_UV(32), MAKE_NORMAL2D(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            Colourless_3D_32_Normal, MAKE_POS3D(32), MAKE_UV(32), MAKE_NORMAL3D(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            Colourless_2D_64_Normal, MAKE_POS2D(64), MAKE_UV(64), MAKE_NORMAL2D(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            Colourless_3D_64_Normal, MAKE_POS3D(64), MAKE_UV(64), MAKE_NORMAL3D(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGB_2D_32_Normal,
            MAKE_POS2D(32),
            MAKE_UV(32),
            MAKE_RGB(32),
            MAKE_NORMAL2D(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGB_3D_32_Normal,
            MAKE_POS3D(32),
            MAKE_UV(32),
            MAKE_RGB(32),
            MAKE_NORMAL3D(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGB_2D_64_Normal,
            MAKE_POS2D(64),
            MAKE_UV(64),
            MAKE_RGB(64),
            MAKE_NORMAL2D(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGB_3D_64_Normal,
            MAKE_POS3D(64),
            MAKE_UV(64),
            MAKE_RGB(64),
            MAKE_NORMAL3D(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGBA_2D_32_Normal,
            MAKE_POS2D(32),
            MAKE_UV(32),
            MAKE_RGBA(32),
            MAKE_NORMAL2D(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGBA_3D_32_Normal,
            MAKE_POS3D(32),
            MAKE_UV(32),
            MAKE_RGBA(32),
            MAKE_NORMAL3D(32)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGBA_2D_64_Normal,
            MAKE_POS2D(64),
            MAKE_UV(64),
            MAKE_RGBA(64),
            MAKE_NORMAL2D(64)
        )

        GEN_MESH_CASE_STRUCT_DEFS_AND_FUNC_DECLS(
            RGBA_3D_64_Normal,
            MAKE_POS3D(64),
            MAKE_UV(64),
            MAKE_RGBA(64),
            MAKE_NORMAL3D(64)
        )

#include "graphics/mesh/common_vertices_undef.hpp"

        void dispose_mesh(const MeshHandles& handles);
        void dispose_mesh(const IndexedMeshHandles& handles);
    }  // namespace graphics
}  // namespace hemlock
namespace hg = hemlock::graphics;

#endif  // __hemlock_graphics_mesh_h

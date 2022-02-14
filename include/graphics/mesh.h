#ifndef __hemlock_graphics_mesh_h
#define __hemlock_graphics_mesh_h

namespace hemlock {
    namespace graphics {
        enum class MeshDataVolatility {
#if defined(HEMLOCK_USING_OPENGL)
            STATIC  = GL_STATIC_DRAW,
            DYNAMIC = GL_DYNAMIC_DRAW,
            STREAM  = GL_STREAM_DRAW
#endif // defined(HEMLOCK_USING_OPENGL)
        };

        struct MeshHandles {
#if defined(HEMLOCK_USING_OPENGL)
            GLuint vao, vbo;
#endif // defined(HEMLOCK_USING_OPENGL)
        };
        struct IndexedMeshHandles : public MeshHandles {
#if defined(HEMLOCK_USING_OPENGL)
            GLuint ibo;
#endif // defined(HEMLOCK_USING_OPENGL)
        };

        // TODO(Matthew): Do we want to support more?
        enum MeshAttribID : GLuint {
            POSITION    = 0,
            COLOUR      = 1,
            UV_COORDS   = 2,
            SENTINEL
        };

        using Vertex_Pos2D_32 = f32v2;
        using Vertex_Pos2D_64 = f64v2;
        using Vertex_Pos3D_32 = f32v3;
        using Vertex_Pos3D_64 = f32v3;
        using Vertex_RGB_32   = f32v3;
        using Vertex_RGB_64   = f64v3;
        using Vertex_RGBA_64  = f64v4;
        using Vertex_RGBA_32  = f32v4;
        using Vertex_UV_32    = f32v2;
        using Vertex_UV_64    = f64v2;

        // TODO(Matthew): RGBA versions.
        struct Vertex2D_32 {
            Vertex_Pos2D_32 position;
            Vertex_RGB_32   colour;
            Vertex_UV_32    uv;
        };
        struct Vertex2D_64 {
            Vertex_Pos2D_64 position;
            Vertex_RGB_64   colour;
            Vertex_UV_64    uv;
        };

        struct Vertex3D_32 {
            Vertex_Pos3D_32 position;
            Vertex_RGB_32   colour;
            Vertex_UV_32    uv;
        };
        struct Vertex3D_64 {
            Vertex_Pos3D_64 position;
            Vertex_RGB_64   colour;
            Vertex_UV_64    uv;
        };

        struct MeshData2D_32 {
            Vertex2D_32* vertices;
            ui32         vertex_count;
        };
        struct MeshData2D_64 {
            Vertex2D_64* vertices;
            ui32         vertex_count;
        };

        struct MeshData3D_32 {
            Vertex3D_32* vertices;
            ui32         vertex_count;
        };
        struct MeshData3D_64 {
            Vertex3D_64* vertices;
            ui32         vertex_count;
        };

        struct IndexData {
            ui32*        indices;
            ui32         index_count;
        };

        struct IndexedMeshData2D_32 : public MeshData2D_32, public IndexData { /* Empty. */ };
        struct IndexedMeshData2D_64 : public MeshData2D_64, public IndexData { /* Empty. */ };
        struct IndexedMeshData3D_32 : public MeshData3D_32, public IndexData { /* Empty. */ };
        struct IndexedMeshData3D_64 : public MeshData3D_64, public IndexData { /* Empty. */ };

        bool upload_mesh(const        MeshData2D_32& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        MeshData2D_64& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        MeshData3D_32& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        MeshData3D_64& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const IndexedMeshData2D_32& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const IndexedMeshData2D_64& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const IndexedMeshData3D_32& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const IndexedMeshData3D_64& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);

        void dispose_mesh(const        MeshHandles& handles);
        void dispose_mesh(const IndexedMeshHandles& handles);
    }
}
namespace hg  = hemlock::graphics;

#endif // __hemlock_graphics_mesh_h

#ifndef __hemlock_graphics_mesh_h
#define __hemlock_graphics_mesh_h

namespace hemlock {
    namespace graphics {
        enum class MeshDataVolatility {
#ifdef HEMLOCK_USING_OPENGL
            STATIC  = GL_STATIC_DRAW,
            DYNAMIC = GL_DYNAMIC_DRAW,
            STREAM  = GL_STREAM_DRAW
#endif // HEMLOCK_USING_OPENGL
        };

        struct MeshHandles {
#ifdef HEMLOCK_USING_OPENGL
            GLuint vao, vbo;
#endif // HEMLOCK_USING_OPENGL
        };
        struct IndexedMeshHandles : public MeshHandles {
#ifdef HEMLOCK_USING_OPENGL
            GLuint ebo;
#endif // HEMLOCK_USING_OPENGL
        };

        struct Vertex_Pos2D_32  : public f32v2 { /* Empty. */ };
        struct Vertex_Pos2D_64  : public f64v2 { /* Empty. */ };
        struct Vertex_Pos3D_32  : public f32v3 { /* Empty. */ };
        struct Vertex_Pos3D_64  : public f32v3 { /* Empty. */ };
        struct Vertex_RGB_32    : public f32v3 { /* Empty. */ };
        struct Vertex_RGB_64    : public f64v3 { /* Empty. */ };
        struct Vertex_RGBA_64   : public f64v4 { /* Empty. */ };
        struct Vertex_RGBA_32   : public f32v4 { /* Empty. */ };
        struct Vertex_UV_32     : public f32v2 { /* Empty. */ };
        struct Vertex_UV_64     : public f64v2 { /* Empty. */ };

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
    }
}
namespace hg  = hemlock::graphics;

#endif // __hemlock_graphics_mesh_h

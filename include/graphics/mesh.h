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

        // TODO(Matthew): Introduce templated version of this.
        //                Essentially compose mesh data out of
        //                structs(?) satisfying some concept
        //                that requires a specification of the
        //                data points required to validly set
        //                the mesh up on the GPU.
        //                  We reeeeaaallly need this, it should
        //                  be doable with the tuple-based index
        //                  trick.

        // TODO(Matthew): Do we want to support more?
        enum MeshAttribID : GLuint {
            POSITION    = 0,
            UV_COORDS   = 1,
            COLOUR      = 2,
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

        struct Colourless_Vertex2D_32 {
            Vertex_Pos2D_32 position;
            Vertex_UV_32    uv;
        };
        struct Colourless_Vertex2D_64 {
            Vertex_Pos2D_64 position;
            Vertex_UV_64    uv;
        };

        struct Colourless_Vertex3D_32 {
            Vertex_Pos3D_32 position;
            Vertex_UV_32    uv;
        };
        struct Colourless_Vertex3D_64 {
            Vertex_Pos3D_64 position;
            Vertex_UV_64    uv;
        };

        struct RGB_Vertex2D_32 {
            Vertex_Pos2D_32 position;
            Vertex_RGB_32   colour;
            Vertex_UV_32    uv;
        };
        struct RGB_Vertex2D_64 {
            Vertex_Pos2D_64 position;
            Vertex_RGB_64   colour;
            Vertex_UV_64    uv;
        };

        struct RGB_Vertex3D_32 {
            Vertex_Pos3D_32 position;
            Vertex_RGB_32   colour;
            Vertex_UV_32    uv;
        };
        struct RGB_Vertex3D_64 {
            Vertex_Pos3D_64 position;
            Vertex_RGB_64   colour;
            Vertex_UV_64    uv;
        };

        struct RGBA_Vertex2D_32 {
            Vertex_Pos2D_32 position;
            Vertex_RGBA_32   colour;
            Vertex_UV_32    uv;
        };
        struct RGBA_Vertex2D_64 {
            Vertex_Pos2D_64 position;
            Vertex_RGBA_64   colour;
            Vertex_UV_64    uv;
        };

        struct RGBA_Vertex3D_32 {
            Vertex_Pos3D_32 position;
            Vertex_RGBA_32   colour;
            Vertex_UV_32    uv;
        };
        struct RGBA_Vertex3D_64 {
            Vertex_Pos3D_64 position;
            Vertex_RGBA_64   colour;
            Vertex_UV_64    uv;
        };

        using Vertex2D_32 = RGB_Vertex2D_32;
        using Vertex2D_64 = RGB_Vertex2D_64;
        using Vertex3D_32 = RGB_Vertex3D_32;
        using Vertex3D_64 = RGB_Vertex3D_64;

        struct Colourless_MeshData2D_32 {
            Colourless_Vertex2D_32* vertices;
            ui32                    vertex_count;
        };
        struct Colourless_MeshData2D_64 {
            Colourless_Vertex2D_64* vertices;
            ui32                    vertex_count;
        };

        struct Colourless_MeshData3D_32 {
            Colourless_Vertex3D_32* vertices;
            ui32                    vertex_count;
        };
        struct Colourless_MeshData3D_64 {
            Colourless_Vertex3D_64* vertices;
            ui32                    vertex_count;
        };

        struct RGB_MeshData2D_32 {
            RGB_Vertex2D_32* vertices;
            ui32             vertex_count;
        };
        struct RGB_MeshData2D_64 {
            RGB_Vertex2D_64* vertices;
            ui32             vertex_count;
        };

        struct RGB_MeshData3D_32 {
            RGB_Vertex3D_32* vertices;
            ui32             vertex_count;
        };
        struct RGB_MeshData3D_64 {
            RGB_Vertex3D_64* vertices;
            ui32             vertex_count;
        };

        struct RGBA_MeshData2D_32 {
            RGBA_Vertex2D_32* vertices;
            ui32              vertex_count;
        };
        struct RGBA_MeshData2D_64 {
            RGBA_Vertex2D_64* vertices;
            ui32              vertex_count;
        };

        struct RGBA_MeshData3D_32 {
            RGBA_Vertex3D_32* vertices;
            ui32              vertex_count;
        };
        struct RGBA_MeshData3D_64 {
            RGBA_Vertex3D_64* vertices;
            ui32              vertex_count;
        };

        struct IndexData {
            ui32*        indices;
            ui32         index_count;
        };

        struct Colourless_IndexedMeshData2D_32 : public Colourless_MeshData2D_32, public IndexData { /* Empty. */ };
        struct Colourless_IndexedMeshData2D_64 : public Colourless_MeshData2D_64, public IndexData { /* Empty. */ };
        struct Colourless_IndexedMeshData3D_32 : public Colourless_MeshData3D_32, public IndexData { /* Empty. */ };
        struct Colourless_IndexedMeshData3D_64 : public Colourless_MeshData3D_64, public IndexData { /* Empty. */ };
        struct RGB_IndexedMeshData2D_32        : public RGB_MeshData2D_32,        public IndexData { /* Empty. */ };
        struct RGB_IndexedMeshData2D_64        : public RGB_MeshData2D_64,        public IndexData { /* Empty. */ };
        struct RGB_IndexedMeshData3D_32        : public RGB_MeshData3D_32,        public IndexData { /* Empty. */ };
        struct RGB_IndexedMeshData3D_64        : public RGB_MeshData3D_64,        public IndexData { /* Empty. */ };
        struct RGBA_IndexedMeshData2D_32       : public RGBA_MeshData2D_32,       public IndexData { /* Empty. */ };
        struct RGBA_IndexedMeshData2D_64       : public RGBA_MeshData2D_64,       public IndexData { /* Empty. */ };
        struct RGBA_IndexedMeshData3D_32       : public RGBA_MeshData3D_32,       public IndexData { /* Empty. */ };
        struct RGBA_IndexedMeshData3D_64       : public RGBA_MeshData3D_64,       public IndexData { /* Empty. */ };

        bool upload_mesh(const        Colourless_MeshData2D_32& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        Colourless_MeshData2D_64& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        Colourless_MeshData3D_32& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        Colourless_MeshData3D_64& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const Colourless_IndexedMeshData2D_32& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const Colourless_IndexedMeshData2D_64& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const Colourless_IndexedMeshData3D_32& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const Colourless_IndexedMeshData3D_64& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);

        bool upload_mesh(const        RGB_MeshData2D_32& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        RGB_MeshData2D_64& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        RGB_MeshData3D_32& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        RGB_MeshData3D_64& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const RGB_IndexedMeshData2D_32& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const RGB_IndexedMeshData2D_64& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const RGB_IndexedMeshData3D_32& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const RGB_IndexedMeshData3D_64& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);

        bool upload_mesh(const        RGBA_MeshData2D_32& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        RGBA_MeshData2D_64& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        RGBA_MeshData3D_32& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const        RGBA_MeshData3D_64& mesh_data, OUT        MeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const RGBA_IndexedMeshData2D_32& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const RGBA_IndexedMeshData2D_64& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const RGBA_IndexedMeshData3D_32& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);
        bool upload_mesh(const RGBA_IndexedMeshData3D_64& mesh_data, OUT IndexedMeshHandles& handles, MeshDataVolatility volatility = MeshDataVolatility::DYNAMIC);

        void dispose_mesh(const        MeshHandles& handles);
        void dispose_mesh(const IndexedMeshHandles& handles);
    }
}
namespace hg  = hemlock::graphics;

#endif // __hemlock_graphics_mesh_h

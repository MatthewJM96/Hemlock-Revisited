#ifndef __hemlock_voxel_graphics_outline_renderer_hpp
#define __hemlock_voxel_graphics_outline_renderer_hpp

#include "graphics/mesh.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;
        class ChunkGrid;

        template <typename Type>
        concept ChunkOutlinePredicate
            = RPredicate<std::tuple<bool, colour4>, Type, hmem::Handle<Chunk>>;

        // TODO(Matthew): Add bit more niceness to rendering, thicker extremity edges
        // and some cross-hatching thinner lines on faces.
        template <ChunkOutlinePredicate Pred>
        class ConditionalChunkOutlineRenderer {
        public:
            ConditionalChunkOutlineRenderer();

            ~ConditionalChunkOutlineRenderer() { /* Empty. */
            }

            /**
             * @brief Initialises voxel outline renderer.
             *
             * @param predicate The predicate with which chunks are
             * evaluated for outlining.
             * @param chunk_grid The chunk grid for which the outlines
             * are rendered.
             */
            void init(Pred predicate, hmem::Handle<ChunkGrid> chunk_grid);
            /**
             * @brief Initialises voxel outline renderer.
             *
             * Essentially just unloads chunk mesh handles
             * as needed.
             */
            void dispose();

            void update(FrameTime) { /* Empty. */
            }

            void draw(FrameTime time);
        protected:
            static hg::MeshHandles   chunk_mesh_handles;
            static std::atomic<ui32> ref_count;

            Delegate<void(Sender, RenderDistanceChangeEvent)>
                handle_render_distance_change;

            Pred m_predicate;

            hmem::Handle<ChunkGrid> m_chunk_grid;

            struct ChunkOutlineCondition {
                f32v3   position;
                colour4 colour;
            };

            ChunkOutlineCondition* m_chunk_outline_conditions;
            ui32                   m_chunk_outline_condition_count;

            GLuint m_instance_vbo;
        };

        const ui32 CHUNK_OUTLINE_VERTEX_COUNT = 24;

        using ChunkOutlineVertex   = hg::Point_3D_32_Vertex;
        using ChunkOutlineMeshData = hg::Point_3D_32_MeshData;

        static ChunkOutlineVertex* const CHUNK_OUTLINE_VERTICES
            = new ChunkOutlineVertex[CHUNK_OUTLINE_VERTEX_COUNT]{
                  { { 0.0f, 0.0f, 0.0f } },
                  { { 0.0f, CHUNK_LENGTH_F, 0.0f } },
                  { { 0.0f, 0.0f, 0.0f } },
                  { { 0.0f, 0.0f, CHUNK_LENGTH_F } },
                  { { 0.0f, 0.0f, 0.0f } },
                  { { CHUNK_LENGTH_F, 0.0f, 0.0f } },
                  { { CHUNK_LENGTH_F, CHUNK_LENGTH_F, CHUNK_LENGTH_F } },
                  { { 0.0f, CHUNK_LENGTH_F, CHUNK_LENGTH_F } },
                  { { CHUNK_LENGTH_F, CHUNK_LENGTH_F, CHUNK_LENGTH_F } },
                  { { CHUNK_LENGTH_F, 0.0f, CHUNK_LENGTH_F } },
                  { { CHUNK_LENGTH_F, CHUNK_LENGTH_F, CHUNK_LENGTH_F } },
                  { { CHUNK_LENGTH_F, CHUNK_LENGTH_F, 0.0f } },
                  { { 0.0f, CHUNK_LENGTH_F, 0.0f } },
                  { { 0.0f, CHUNK_LENGTH_F, CHUNK_LENGTH_F } },
                  { { 0.0f, CHUNK_LENGTH_F, 0.0f } },
                  { { CHUNK_LENGTH_F, CHUNK_LENGTH_F, 0.0f } },
                  { { CHUNK_LENGTH_F, 0.0f, CHUNK_LENGTH_F } },
                  { { CHUNK_LENGTH_F, 0.0f, 0.0f } },
                  { { CHUNK_LENGTH_F, 0.0f, CHUNK_LENGTH_F } },
                  { { 0.0f, 0.0f, CHUNK_LENGTH_F } },
                  { { 0.0f, CHUNK_LENGTH_F, CHUNK_LENGTH_F } },
                  { { 0.0f, 0.0f, CHUNK_LENGTH_F } },
                  { { CHUNK_LENGTH_F, 0.0f, 0.0f } },
                  { { CHUNK_LENGTH_F, CHUNK_LENGTH_F, 0.0f } }
              };

        const ChunkOutlineMeshData CHUNK_OUTLINE_MESH
            = { CHUNK_OUTLINE_VERTICES, CHUNK_OUTLINE_VERTEX_COUNT };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/graphics/outline_renderer.inl"

#endif  // __hemlock_voxel_graphics_outline_renderer_hpp

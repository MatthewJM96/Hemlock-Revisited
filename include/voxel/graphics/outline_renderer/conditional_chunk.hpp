#ifndef __hemlock_voxel_graphics_outline_renderer_conditional_chunk_hpp
#define __hemlock_voxel_graphics_outline_renderer_conditional_chunk_hpp

#include "state.hpp"

namespace hemlock {
    namespace voxel {
        template <ChunkDecorator... Decorations>
        class ChunkGrid;

        // TODO(Matthew): Add bit more niceness to rendering, thicker extremity edges
        // and some cross-hatching thinner lines on faces.
        template <OutlinePredicate Pred, ChunkDecorator... Decorations>
        class ConditionalChunkOutlineRenderer {
        public:
            using _ChunkGrid = ChunkGrid<Decorations...>;

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
            void init(Pred predicate, hmem::Handle<_ChunkGrid> chunk_grid);
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

            hmem::Handle<_ChunkGrid> m_chunk_grid;

            OutlineData* m_chunk_outline_conditions;
            ui32         m_chunk_outline_condition_count;

            GLuint m_instance_vbo;
        };

        const ui32 CHUNK_OUTLINE_VERTEX_COUNT = 24;

        static const OutlineVertex CHUNK_OUTLINE_VERTICES[CHUNK_OUTLINE_VERTEX_COUNT]
            = { { { 0.0f, 0.0f, 0.0f } },
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
                { { CHUNK_LENGTH_F, CHUNK_LENGTH_F, 0.0f } } };

        static const OutlineMeshData CHUNK_OUTLINE_MESH
            = { &CHUNK_OUTLINE_VERTICES[0], CHUNK_OUTLINE_VERTEX_COUNT };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "conditional_chunk.inl"

#endif  // __hemlock_voxel_graphics_outline_renderer_conditional_chunk_hpp

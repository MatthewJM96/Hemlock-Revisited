#ifndef __hemlock_voxel_graphics_outline_renderer_h
#define __hemlock_voxel_graphics_outline_renderer_h

#include "graphics/mesh.h"
#include "voxel/chunk/constants.hpp"
#include "voxel/chunk/events.hpp"

namespace hemlock {
    namespace voxel {
        struct Chunk;
        class ChunkGrid;

        template <typename Type>
        concept OutlinePredicate
            = RPredicate<std::tuple<bool, colour4>, Type, hmem::Handle<Chunk>>;

        struct OutlineData {
            f32v3   position;
            colour4 colour;
        };

        // TODO(Matthew): Add bit more niceness to rendering, thicker extremity edges
        // and some cross-hatching thinner lines on faces.
        template <OutlinePredicate Pred>
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

            OutlineData* m_chunk_outline_conditions;
            ui32         m_chunk_outline_condition_count;

            GLuint m_instance_vbo;
        };

        class BlockOutlineRenderer {
        public:
            BlockOutlineRenderer();

            ~BlockOutlineRenderer() { /* Empty. */
            }

            /**
             * @brief Initialises voxel outline renderer.
             *
             * Essentially just loads chunk mesh handles
             * as needed.
             */
            void init();
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

            size_t add_outline(OutlineData&& outline);

            bool modify_outline(size_t outline_id, OutlineData&& outline);

            bool remove_outline(size_t outline_id);
        protected:
            static hg::MeshHandles   block_mesh_handles;
            static std::atomic<ui32> ref_count;

            using BlockOutlines    = std::vector<OutlineData>;
            using BlockOutlineRefs = std::unordered_map<size_t, size_t>;

            BlockOutlines    m_block_outlines;
            BlockOutlineRefs m_block_outline_refs;
            size_t           m_next_outline_id;

            size_t m_last_outline_count;

            GLuint m_instance_vbo;
        };

        const ui32 CHUNK_OUTLINE_VERTEX_COUNT = 24;
        const ui32 BLOCK_OUTLINE_VERTEX_COUNT = 24;

        using OutlineVertex   = hg::Point_3D_32_Vertex;
        using OutlineMeshData = hg::ConstPoint_3D_32_MeshData;

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

        static const OutlineVertex BLOCK_OUTLINE_VERTICES[BLOCK_OUTLINE_VERTEX_COUNT]
            = { { { 0.0f, 0.0f, 0.0f } }, { { 0.0f, 1.0f, 0.0f } },
                { { 0.0f, 0.0f, 0.0f } }, { { 0.0f, 0.0f, 1.0f } },
                { { 0.0f, 0.0f, 0.0f } }, { { 1.0f, 0.0f, 0.0f } },
                { { 1.0f, 1.0f, 1.0f } }, { { 0.0f, 1.0f, 1.0f } },
                { { 1.0f, 1.0f, 1.0f } }, { { 1.0f, 0.0f, 1.0f } },
                { { 1.0f, 1.0f, 1.0f } }, { { 1.0f, 1.0f, 0.0f } },
                { { 0.0f, 1.0f, 0.0f } }, { { 0.0f, 1.0f, 1.0f } },
                { { 0.0f, 1.0f, 0.0f } }, { { 1.0f, 1.0f, 0.0f } },
                { { 1.0f, 0.0f, 1.0f } }, { { 1.0f, 0.0f, 0.0f } },
                { { 1.0f, 0.0f, 1.0f } }, { { 0.0f, 0.0f, 1.0f } },
                { { 0.0f, 1.0f, 1.0f } }, { { 0.0f, 0.0f, 1.0f } },
                { { 1.0f, 0.0f, 0.0f } }, { { 1.0f, 1.0f, 0.0f } } };

        static const OutlineMeshData BLOCK_OUTLINE_MESH
            = { &BLOCK_OUTLINE_VERTICES[0], BLOCK_OUTLINE_VERTEX_COUNT };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/graphics/outline_renderer.inl"

#endif  // __hemlock_voxel_graphics_outline_renderer_h

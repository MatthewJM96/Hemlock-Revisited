#ifndef __hemlock_voxel_graphics_outline_renderer_block_h
#define __hemlock_voxel_graphics_outline_renderer_block_h

#include "state.hpp"

namespace hemlock {
    namespace voxel {
        struct Chunk;
        class ChunkGrid;

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

        const ui32 BLOCK_OUTLINE_VERTEX_COUNT = 24;

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

#endif  // __hemlock_voxel_graphics_outline_renderer_block_h

#ifndef __hemlock_voxel_graphics_outline_renderer_voxel_h
#define __hemlock_voxel_graphics_outline_renderer_voxel_h

#include "state.hpp"

namespace hemlock {
    namespace voxel {
        class VoxelOutlineRenderer {
        public:
            VoxelOutlineRenderer();

            ~VoxelOutlineRenderer() { /* Empty. */
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
            static hg::MeshHandles   voxel_mesh_handles;
            static std::atomic<ui32> ref_count;

            using VoxelOutlines    = std::vector<OutlineData>;
            using VoxelOutlineRefs = std::unordered_map<size_t, size_t>;

            VoxelOutlines    m_voxel_outlines;
            VoxelOutlineRefs m_voxel_outline_refs;
            size_t           m_next_outline_id;

            size_t m_last_outline_count;

            GLuint m_instance_vbo;
        };

        const ui32 VOXEL_OUTLINE_VERTEX_COUNT = 24;

        static const OutlineVertex VOXEL_OUTLINE_VERTICES[VOXEL_OUTLINE_VERTEX_COUNT]
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

        static const OutlineMeshData VOXEL_OUTLINE_MESH
            = { &VOXEL_OUTLINE_VERTICES[0], VOXEL_OUTLINE_VERTEX_COUNT };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_graphics_outline_renderer_voxel_h

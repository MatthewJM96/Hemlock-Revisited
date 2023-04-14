#ifndef __hemlock_voxel_graphics_outline_renderer_navmesh_h
#define __hemlock_voxel_graphics_outline_renderer_navmesh_h

#include "state.hpp"

#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        class NavmeshOutlineRenderer {
        public:
            NavmeshOutlineRenderer();

            ~NavmeshOutlineRenderer() { /* Empty. */
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

            void register_chunk(hmem::Handle<Chunk> chunk);
        protected:
            Delegate<void(Sender)> handle_chunk_navmesh_change;
            Delegate<void(Sender)> handle_chunk_unload;

            hg::MeshHandles m_navmesh_mesh_handles;

            struct NavmeshOutlineData {
                f32v3 start, end;
            };

            using NavmeshOutlines    = std::vector<NavmeshOutlineData>;
            using NavmeshOutlineRefs = std::unordered_map<size_t, size_t>;

            NavmeshOutlines    m_block_outlines;
            NavmeshOutlineRefs m_block_outline_refs;
            size_t             m_next_outline_id;

            size_t m_last_outline_count;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_graphics_outline_renderer_navmesh_h

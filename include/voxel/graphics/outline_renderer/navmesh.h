#ifndef __hemlock_voxel_graphics_outline_renderer_navmesh_h
#define __hemlock_voxel_graphics_outline_renderer_navmesh_h

#include "state.hpp"

#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        class NavmeshOutlineRenderer {
        protected:
            struct NavmeshOutlineData {
                f32v3 start, end;
            };

            struct NavmeshOutlines {
                std::vector<NavmeshOutlineData> outlines;
                std::atomic<bool>               is_dirty;
                std::atomic<bool>               is_dead;
                bool                            is_updated;
                size_t                          last_size;
                hg::MeshHandles                 mesh_handles;
                hmem::WeakHandle<Chunk>         chunk;

                NavmeshOutlines() :
                    outlines{},
                    is_dirty{},
                    is_dead{},
                    is_updated(false),
                    last_size(0),
                    mesh_handles{},
                    chunk{} {
                    /* Empty. */
                }

                NavmeshOutlines(const NavmeshOutlines& rhs) {
                    outlines = rhs.outlines;
                    is_dirty.store(rhs.is_dirty.load());
                    is_dead.store(rhs.is_dead.load());
                    is_updated   = rhs.is_updated;
                    last_size    = rhs.last_size;
                    mesh_handles = rhs.mesh_handles;
                    chunk        = rhs.chunk;
                }
            };

            using NavmeshOutlinesPerChunk
                = std::unordered_map<ChunkID, NavmeshOutlines>;
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

            void update(FrameTime);

            void draw(FrameTime);

            void register_chunk(hmem::Handle<Chunk> chunk);
        protected:
            void __calculate_outlines(NavmeshOutlines& navmesh);

            Delegate<void(Sender)> handle_chunk_navmesh_change;
            Delegate<void(Sender)> handle_chunk_unload;

            GLuint m_navmesh_outline_vao;

            NavmeshOutlinesPerChunk m_navmesh_outlines;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_graphics_outline_renderer_navmesh_h

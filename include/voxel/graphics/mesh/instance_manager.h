#ifndef __hemlock_voxel_graphics_mesh_instance_manager_h
#define __hemlock_voxel_graphics_mesh_instance_manager_h

#include "voxel/block.hpp"
#include "voxel/chunk/constants.hpp"

namespace hemlock {
    namespace voxel {
        struct ChunkInstanceData {
            f32v3 translation, scaling;
        };

        struct ChunkInstance {
            ChunkInstanceData* data;
            ui32               count;
        };

        // TODO(Matthew): Maybe we want to make instances out of smaller pages and
        // expand on demand.
        using ChunkInstanceDataPager = hmem::Pager<ChunkInstanceData, CHUNK_VOLUME, 3>;
        using ChunkBlockPager        = hmem::Pager<Block, CHUNK_VOLUME, 3>;

        class ChunkInstanceManager {
        public:
            ChunkInstanceManager();

            ~ChunkInstanceManager() { /* Empty. */
            }

            void init(hmem::Handle<ChunkInstanceDataPager> data_pager);
            void dispose();

            ChunkInstance&       get(std::unique_lock<std::shared_mutex>& lock);
            const ChunkInstance& get(std::shared_lock<std::shared_mutex>& lock);

            void generate_buffer();
            void free_buffer();
        protected:
            hmem::Handle<ChunkInstanceDataPager> m_data_pager;

            std::shared_mutex m_mutex;
            ChunkInstance     m_instance;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_graphics_mesh_instance_manager_h

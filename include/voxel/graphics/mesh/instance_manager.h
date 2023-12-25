#ifndef __hemlock_voxel_graphics_mesh_instance_manager_h
#define __hemlock_voxel_graphics_mesh_instance_manager_h

#include "thread/resource_guard.hpp"
#include "voxel/chunk/constants.hpp"
#include "voxel/state.hpp"

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

        class ChunkInstanceManager : public hthread::ResourceGuard<ChunkInstance> {
        public:
            void init(hmem::Handle<ChunkInstanceDataPager> data_pager);
            void dispose();

            void generate_buffer();
            void free_buffer();
        protected:
            hmem::Handle<ChunkInstanceDataPager> m_data_pager;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_graphics_mesh_instance_manager_h

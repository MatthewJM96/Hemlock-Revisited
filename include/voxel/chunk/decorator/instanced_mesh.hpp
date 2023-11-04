#ifndef __hemlock_voxel_chunk_decorator_instanced_mesh_hpp
#define __hemlock_voxel_chunk_decorator_instanced_mesh_hpp

#include "voxel/graphics/mesh/instance_manager.h"

namespace hemlock {
    namespace voxel {
        /**
         * @brief
         */
        struct InstancedMeshDecorator {
            void
            set_instance_pager(hmem::Handle<ChunkInstanceDataPager> instance_data_pager
            );

            ChunkInstanceManager instance;
        protected:
            hmem::Handle<ChunkInstanceDataPager> m_instance_data_pager;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_decorator_instanced_mesh_hpp

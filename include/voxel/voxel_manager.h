#ifndef __hemlock_voxel_voxel_manager_h
#define __hemlock_voxel_voxel_manager_h

#include "thread/resource_guard.hpp"
#include "voxel/chunk/constants.hpp"
#include "voxel/state.hpp"

namespace hemlock {
    namespace voxel {
        using ChunkVoxelPager = hmem::Pager<Voxel, CHUNK_VOLUME, 3>;

        class VoxelManager : public hthread::ResourceGuard<Voxel*> {
        public:
            void init(hmem::Handle<ChunkVoxelPager> voxel_pager);
            void dispose();

            void generate_buffer();
            void free_buffer();
        protected:
            hmem::Handle<ChunkVoxelPager> m_voxel_pager;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_voxel_manager_h

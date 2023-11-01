#ifndef __hemlock_voxel_block_manager_h
#define __hemlock_voxel_block_manager_h

#include "thread/resource_guard.hpp"
#include "voxel/block.hpp"

namespace hemlock {
    namespace voxel {
        using ChunkBlockPager = hmem::Pager<Block, CHUNK_VOLUME, 3>;

        class BlockManager : public hthread::ResourceGuard<Block*> {
        public:
            void init(hmem::Handle<ChunkBlockPager> block_pager);
            void dispose();

            void generate_buffer();
            void free_buffer();
        protected:
            hmem::Handle<ChunkBlockPager> m_block_pager;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_block_manager_h

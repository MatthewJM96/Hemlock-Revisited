#ifndef __hemlock_voxel_ai_navmesh_navmesh_manager_h
#define __hemlock_voxel_ai_navmesh_navmesh_manager_h

#include "thread/resource_guard.hpp"

#include "state.hpp"

namespace hemlock {
    namespace voxel {
        namespace ai {
            using ChunkNavmeshPager = hmem::Pager<ChunkNavmesh, 1, 3>;

            class ChunkNavmeshManager : public hthread::ResourceGuard<ChunkNavmesh*> {
            public:
                void init(hmem::Handle<ChunkNavmeshPager> navmesh_pager);
                void dispose();

                void generate_buffer();
                void free_buffer();
            protected:
                hmem::Handle<ChunkNavmeshPager> m_navmesh_pager;
            };
        }  // namespace ai
    }      // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_ai_navmesh_navmesh_manager_h

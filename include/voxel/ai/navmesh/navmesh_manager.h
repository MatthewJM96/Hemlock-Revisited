#ifndef __hemlock_voxel_ai_navmesh_navmesh_manager_h
#define __hemlock_voxel_ai_navmesh_navmesh_manager_h

#include "memory/resource_manager.hpp"

#include "state.hpp"

namespace hemlock {
    namespace voxel {
        namespace ai {
            using ChunkNavmeshPaged = hmem::PagedResource<ChunkNavmesh>;

            using ChunkNavmeshPager = hmem::Pager<ChunkNavmesh, 1, 3>;

            class ChunkNavmeshManager :
                public hmem::ResourceManager<ChunkNavmesh, ChunkNavmeshPager> {
            public:
                virtual void generate_buffer() override final;
                virtual void free_buffer() override final;
            };
        }  // namespace ai
    }      // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_ai_navmesh_navmesh_manager_h

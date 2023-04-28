#ifndef __hemlock_voxel_ai_navmesh_navmesh_manager_hpp
#define __hemlock_voxel_ai_navmesh_navmesh_manager_hpp

#include "memory/resource_manager.hpp"

#include "state.hpp"

namespace hemlock {
    namespace voxel {
        namespace ai {
            using ChunkNavmeshPaged = hmem::PagedResource<ChunkNavmesh>;

            using ChunkNavmeshPager = hmem::Pager<ChunkNavmesh, 1, 3>;

            using ChunkNavmeshManager
                = hmem::ResourceManager<ChunkNavmesh, ChunkNavmeshPager>;
        }  // namespace ai
    }      // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_ai_navmesh_navmesh_manager_hpp

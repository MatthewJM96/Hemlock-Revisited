#ifndef __hemlock_voxel_graphics_mesh_instance_manager_h
#define __hemlock_voxel_graphics_mesh_instance_manager_h

#include "memory/resource_manager.hpp"
#include "voxel/block.hpp"
#include "voxel/chunk/constants.hpp"

namespace hemlock {
    namespace voxel {
        struct ChunkInstanceData {
            f32v3 translation, scaling;
        };

        using ChunkInstance = hmem::PagedResource<ChunkInstanceData>;

        // TODO(Matthew): Maybe we want to make instances out of smaller pages and
        // expand on demand.
        using ChunkInstancePager = hmem::Pager<ChunkInstanceData, CHUNK_VOLUME, 3>;
        using ChunkBlockPager    = hmem::Pager<Block, CHUNK_VOLUME, 3>;

        using ChunkInstanceManager
            = hmem::ResourceManager<ChunkInstanceData, ChunkInstancePager>;
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_graphics_mesh_instance_manager_h

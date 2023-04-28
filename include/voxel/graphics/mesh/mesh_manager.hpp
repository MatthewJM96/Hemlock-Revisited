#ifndef __hemlock_voxel_graphics_mesh_mesh_manager_h
#define __hemlock_voxel_graphics_mesh_mesh_manager_h

#include "memory/resource_manager.hpp"
#include "voxel/block.hpp"
#include "voxel/chunk/constants.hpp"

namespace hemlock {
    namespace voxel {
        struct ChunkMeshData {
            f32v3 translation, scaling;
        };

        using ChunkMesh  = hmem::PagedResource<ChunkMeshData>;
        using BlockPaged = hmem::PagedResource<Block>;

        // TODO(Matthew): Maybe we want to make instances out of smaller pages and
        // expand on demand.
        using ChunkMeshPager  = hmem::Pager<ChunkMeshData, CHUNK_VOLUME, 3>;
        using ChunkBlockPager = hmem::Pager<Block, CHUNK_VOLUME, 3>;

        using ChunkMeshManager  = hmem::ResourceManager<ChunkMeshData, ChunkMeshPager>;
        using ChunkBlockManager = hmem::ResourceManager<Block, ChunkBlockPager>;
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_graphics_mesh_mesh_manager_h

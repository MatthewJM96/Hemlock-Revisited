#ifndef __hemlock_voxel_chunk_mesh_mesh_task_hpp
#define __hemlock_voxel_chunk_mesh_mesh_task_hpp

namespace hemlock {
    namespace voxel {
        /**
         * @brief Determines if two blocks are of the same mesheable kind.
         *
         * The first block pointer is to the comparator block, while the second
         * is the actual block at the specified position within the specified
         * chunk.
         */
        using ChunkMeshStrategy = Delegate<bool(const Block*, const Block*, BlockChunkPosition, Chunk*)>;
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_mesh_mesh_task_hpp

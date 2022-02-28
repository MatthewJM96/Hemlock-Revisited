#ifndef __hemlock_voxel_chunk_mesh_mesh_task_hpp
#define __hemlock_voxel_chunk_mesh_mesh_task_hpp

namespace hemlock {
    namespace voxel {
        /**
         * @brief Defines a struct whose opeartor() determines if two block
         * are of the same mesheable kind.
         *
         * The first block pointer is to the comparator block, while the second
         * is the actual block at the specified position within the specified
         * chunk.
         */
        template <typename ComparatorCandidate>
        concept ChunkMeshComparator = requires (
             ComparatorCandidate s,
                    const Block* b,
              BlockChunkPosition p,
                          Chunk* c
        ) {
            { s.operator()(b, b, p, c) } -> std::same_as<bool>;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_mesh_mesh_task_hpp

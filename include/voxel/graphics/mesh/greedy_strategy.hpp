#ifndef __hemlock_voxel_graphics_mesh_greedy_strategy_hpp
#define __hemlock_voxel_graphics_mesh_greedy_strategy_hpp

namespace hemlock {
    namespace voxel {
        template <hvox::IdealBlockComparator MeshComparator>
        struct GreedyMeshStrategy {
            bool can_run(hmem::Handle<ChunkGrid> chunk_grid, hmem::Handle<Chunk> chunk) const;

            void operator()(hmem::Handle<ChunkGrid> chunk_grid, hmem::Handle<Chunk> chunk) const;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/graphics/mesh/greedy_strategy.inl"

#endif  // __hemlock_voxel_graphics_mesh_greedy_strategy_hpp

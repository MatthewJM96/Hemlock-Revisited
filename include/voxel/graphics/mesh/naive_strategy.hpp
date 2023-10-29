#ifndef __hemlock_voxel_graphics_mesh_naive_strategy_hpp
#define __hemlock_voxel_graphics_mesh_naive_strategy_hpp

#include "voxel/predicate.hpp"

namespace hemlock {
    namespace voxel {
        template <hvox::IdealBlockComparator MeshComparator>
        struct NaiveMeshStrategy {
            bool can_run(hmem::Handle<ChunkGrid> chunk_grid, entt::entity chunk) const;

            void
            operator()(hmem::Handle<ChunkGrid> chunk_grid, entt::entity chunk) const;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "naive_strategy.inl"

#endif  // __hemlock_voxel_graphics_mesh_naive_strategy_hpp

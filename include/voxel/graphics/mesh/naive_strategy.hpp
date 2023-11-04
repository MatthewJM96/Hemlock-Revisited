#ifndef __hemlock_voxel_graphics_mesh_naive_strategy_hpp
#define __hemlock_voxel_graphics_mesh_naive_strategy_hpp

#include "voxel/predicate.hpp"

namespace hemlock {
    namespace voxel {
        template <
            hvox::IdealBlockComparator MeshComparator,
            ChunkDecorator... Decorations>
        struct NaiveMeshStrategy {
            bool can_run(
                hmem::Handle<ChunkGrid<Decorations...>> chunk_grid,
                hmem::Handle<Chunk<Decorations...>>     chunk
            ) const;

            void operator()(
                hmem::Handle<ChunkGrid<Decorations...>> chunk_grid,
                hmem::Handle<Chunk<Decorations...>>     chunk
            ) const;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "naive_strategy.inl"

#endif  // __hemlock_voxel_graphics_mesh_naive_strategy_hpp

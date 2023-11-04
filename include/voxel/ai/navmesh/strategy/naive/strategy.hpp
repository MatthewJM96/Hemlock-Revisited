#ifndef __hemlock_voxel_ai_navmesh_strategy_naive_strategy_hpp
#define __hemlock_voxel_ai_navmesh_strategy_naive_strategy_hpp

#include "voxel/predicate.hpp"

namespace hemlock {
    namespace voxel {
        template <ChunkDecorator... Decorations>
        struct Chunk;

        namespace ai {
            template <hvox::IdealBlockConstraint IsSolid, ChunkDecorator... Decorations>
            struct NaiveNavmeshStrategy {
                void do_bulk(
                    hmem::Handle<ChunkGrid<Decorations...>> chunk_grid,
                    hmem::Handle<Chunk<Decorations...>>     chunk
                ) const;

                void do_stitch(
                    hmem::Handle<ChunkGrid<Decorations...>> chunk_grid,
                    hmem::Handle<Chunk<Decorations...>>     chunk
                ) const;
            };
        }  // namespace ai
    }      // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "strategy.inl"

#endif  // __hemlock_voxel_ai_navmesh_strategy_naive_strategy_hpp

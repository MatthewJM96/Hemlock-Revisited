#ifndef __hemlock_voxel_ai_navmesh_naive_strategy_hpp
#define __hemlock_voxel_ai_navmesh_naive_strategy_hpp

#include "voxel/predicate.hpp"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        namespace ai {
            template <hvox::IdealBlockConstraint IsSolid>
            struct NaiveNavmeshStrategy {
                void do_bulk(hmem::Handle<ChunkGrid> chunk_grid, hmem::Handle<Chunk> chunk) const;

                void do_stitch(hmem::Handle<ChunkGrid> chunk_grid, hmem::Handle<Chunk> chunk) const;
            };
        }  // namespace ai
    }      // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "navmesh_strategy.inl"

#endif  // __hemlock_voxel_ai_navmesh_naive_strategy_hpp

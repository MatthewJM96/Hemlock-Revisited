#ifndef __hemlock_voxel_ai_navmesh_view_hpp
#define __hemlock_voxel_ai_navmesh_view_hpp

#include "algorithm/acs/graph/view.hpp"
#include "state.hpp"
#include "voxel/chunk/decorator/decorator.hpp"
#include "voxel/chunk/grid.hpp"

namespace hemlock {
    namespace voxel {
        namespace ai {
            template <ChunkDecorator... Decorations>
            class ChunkGridGraphMapView :
                public halgo::GraphMapView<ChunkNavmeshNode, false> {
            public:
                void init(hmem::WeakHandle<ChunkGrid<Decorations...>> grid) {
                    m_grid = grid;
                }

                std::tuple<ChunkNavmeshVertexDescriptor, ChunkNavmesh*>
                vertex(ChunkNavmeshNode node) override final {
                    auto grid = m_grid.lock();
                    if (grid == nullptr) return { 0, nullptr };

                    auto chunk = grid->chunk(node.chunk_pos);
                    if (chunk == nullptr) return { 0, nullptr };

                    // TODO(Matthew): need to actually lock navmesh with a lock in
                    // caller
                    //                scope as we can't otherwise allow writing to the
                    //                navmesh data, which we want to be able to do. Not
                    //                so easy as the API needs to be general, and
                    //                locking isn't always required.
                    std::unique_lock<std::shared_mutex> lock;
                    auto navmesh = chunk->navmesh.get(lock);

                    return { navmesh->coord_vertex_map[node], navmesh };
                }
            protected:
                hmem::WeakHandle<ChunkGrid<Decorations...>> m_grid;
            };
        }  // namespace ai
    }      // namespace voxel
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#endif  // __hemlock_voxel_ai_navmesh_view_hpp

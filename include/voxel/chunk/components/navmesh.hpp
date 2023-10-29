#ifndef __hemlock_voxel_chunk_components_navmesh_hpp
#define __hemlock_voxel_chunk_components_navmesh_hpp

#include "voxel/ai/navmesh.hpp"
#include "voxel/chunk/state.hpp"

namespace hemlock {
    namespace voxel {
        struct ChunkNavmeshComponent {
            ChunkNavmeshComponent() :
                navmesh_stitch({}), bulk_navmeshing({}), navmeshing({}) {
                // TODO(Matthew): how are we going to do this? sender as pointer to
                //                entity stored in grid? what about how to know which
                //                registry that entity is stored in?
                //                  note that we probably don't want to guarantee
                //                  pointer stability of 'this' as per EnTT docs, and
                //                  anyway using 'this' would mean not giving listener
                //                  access to any other components of the chunk.
                on_navmesh_change.set_sender(Sender());
            }

            std::shared_mutex navmesh_mutex;
            ai::ChunkNavmesh  navmesh;

            struct {
                std::atomic<ChunkState> right, top, front, above_left, above_right,
                    above_front, above_back, above_and_across_left,
                    above_and_across_right, above_and_across_front,
                    above_and_across_back;
            } navmesh_stitch;

            std::atomic<ChunkState> bulk_navmeshing, navmeshing;

            Event<> on_navmesh_change;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_components_navmesh_hpp

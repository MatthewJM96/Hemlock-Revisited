#ifndef __hemlock_voxel_chunk_components_lodable_hpp
#define __hemlock_voxel_chunk_components_lodable_hpp

#include "voxel/block.hpp"
#include "voxel/chunk/events/lod_change.hpp"

namespace hemlock {
    namespace voxel {
        /**
         * Represents a chunk that can be rendered are multiple LODs. The LOD level is
         * represented by an integer 0-255, this should be sufficient for any use-case.
         */
        struct ChunkLODComponent {
            ChunkLODComponent() : lod_level({}) {
                // TODO(Matthew): how are we going to do this? sender as pointer to
                //                entity stored in grid? what about how to know which
                //                registry that entity is stored in?
                //                  note that we probably don't want to guarantee
                //                  pointer stability of 'this' as per EnTT docs, and
                //                  anyway using 'this' would mean not giving listener
                //                  access to any other components of the chunk.
                on_lod_change.set_sender(Sender());
            }

            std::atomic<LODLevel> lod_level;

            Event<LODChangeEvent> on_lod_change;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_components_lodable_hpp

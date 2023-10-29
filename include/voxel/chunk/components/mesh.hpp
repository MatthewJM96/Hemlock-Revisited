#ifndef __hemlock_voxel_chunk_components_mesh_hpp
#define __hemlock_voxel_chunk_components_mesh_hpp

#include "voxel/chunk/state.hpp"
#include "voxel/graphics/mesh/instance_manager.h"

namespace hemlock {
    namespace voxel {
        struct ChunkMeshComponent {
            ChunkMeshComponent(hmem::Handle<ChunkInstanceDataPager> instance_data_pager
            ) :
                meshing({}), mesh_uploading({}) {
                instance.init(instance_data_pager);

                // TODO(Matthew): how are we going to do this? sender as pointer to
                //                entity stored in grid? what about how to know which
                //                registry that entity is stored in?
                //                  note that we probably don't want to guarantee
                //                  pointer stability of 'this' as per EnTT docs, and
                //                  anyway using 'this' would mean not giving listener
                //                  access to any other components of the chunk.
                on_mesh_change.set_sender(Sender());
            }

            ~ChunkMeshComponent() { instance.dispose(); }

            ChunkInstanceManager instance;

            std::atomic<ChunkState> meshing, mesh_uploading;

            Event<> on_mesh_change;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_components_mesh_hpp

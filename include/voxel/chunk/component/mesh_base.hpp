#ifndef __hemlock_voxel_chunk_component_mesh_base_hpp
#define __hemlock_voxel_chunk_component_mesh_base_hpp

namespace hemlock {
    namespace voxel {
        /**
         * @brief Holds general mesh state. That is to say the event used to notify of
         * any mesh changes in the chunk in-question.
         *
         * NOTE: This is NOT a sufficient component to be used as a mesh component
         * without adding further state to hold the associated mesh data.
         */
        struct ChunkMeshComponentBase {
            Event<> on_mesh_change;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_component_mesh_base_hpp

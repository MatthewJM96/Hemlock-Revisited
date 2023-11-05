#ifndef __hemlock_voxel_chunk_component_instanced_mesh_hpp
#define __hemlock_voxel_chunk_component_instanced_mesh_hpp

#include "voxel/chunk/component/mesh_base.hpp"
#include "voxel/graphics/mesh/instance_manager.h"

namespace hemlock {
    namespace voxel {
        /**
         * @brief Holds state necessary to impart on a chunk an instanced mesh.
         */
        struct ChunkInstancedMeshComponent : public ChunkMeshComponentBase {
            ChunkInstanceManager instance;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_component_instanced_mesh_hpp

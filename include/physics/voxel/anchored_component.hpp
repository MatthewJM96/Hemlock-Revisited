#ifndef __hemlock_physics_voxel_anchored_component_hpp
#define __hemlock_physics_voxel_anchored_component_hpp

#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        class ChunkGrid;
    }

    namespace physics {

        struct AnchoredComponent {
            hmem::WeakHandle<voxel::ChunkGrid> chunk_grid;
            hvox::EntityWorldPosition          position;
        };

    }  // namespace physics
}  // namespace hemlock
namespace hphys = hemlock::physics;

#endif  // __hemlock_physics_voxel_anchored_component_hpp

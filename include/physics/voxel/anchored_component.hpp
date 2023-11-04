#ifndef __hemlock_physics_voxel_anchored_component_hpp
#define __hemlock_physics_voxel_anchored_component_hpp

#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        template <ChunkDecorator... Decorations>
        class ChunkGrid;
    }

    namespace physics {
        template <hvox::ChunkDecorator... Decorations>
        struct AnchoredComponent {
            hmem::WeakHandle<voxel::ChunkGrid<Decorations...>> chunk_grid;
            hvox::EntityWorldPosition                          position;
        };

    }  // namespace physics
}  // namespace hemlock
namespace hphys = hemlock::physics;

#endif  // __hemlock_physics_voxel_anchored_component_hpp

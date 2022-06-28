#ifndef __hemlock_physics_common_components_hpp
#define __hemlock_physics_common_components_hpp

namespace hemlock {
    namespace physics {
        // TODO(Matthew): fixed-point coordinates here as in voxel coordinate system, likewise for time.

        using PositionComponent = f32v3;

        // TODO(Matthew): get bullet working and incorporate shapes here.
        struct CollidableComponent {
            f32v3 size;
        };

        struct MovableComponent {
            f32v3 velocity;
            
        };
    }
}
namespace hphys = hemlock::physics;

#endif // __hemlock_physics_common_components_hpp

#ifndef __hemlock_physics_common_components_hpp
#define __hemlock_physics_common_components_hpp

namespace hemlock {
    namespace physics {
        // TODO(Matthew): fixed-point coordinates here as in voxel coordinate system, likewise for time.
        // TODO(Matthew): All dynamic (extended?) components can be anchored to some voxel (chunk) grid
        //                their global coordinates being in fixed-point on that grid, with floating-point
        //                being used by Bullet relative to those dynamic (extended?) components' origins.
        //                  I.e. we figure out what parts of the grid they may be colliding with at all,
        //                  generate boxes for those bits of the grid in the dynamic (extended?) components'
        //                  coordinate system (in floating-point), and then ask Bullet how things react
        //                  before returning to the voxel grid coordinate system. We thus avoid the issues
        //                  with floating point of going outside the reasonable range of precision we get
        //                  for small numbers.


        struct StaticComponent {
            f32v3 position;
        };

        // TODO(Matthew): get bullet working and incorporate shapes here.
        // TODO(Matthew): can use StaticComponent AND NOT DynamicComponent group/view,
        //                but could just inheriting StaticComponent make sense here?
        struct DynamicComponent {
            f32v3 velocity;
        };
        struct DynamicCollidableComponent {
            f32v3 size;
        };

    }
}
namespace hphys = hemlock::physics;

#endif // __hemlock_physics_common_components_hpp

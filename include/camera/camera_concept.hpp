#ifndef __hemlock_camera_camera_concept_hpp
#define __hemlock_camera_camera_concept_hpp

namespace hemlock {
    namespace app {
        class WindowBase;
    }

    namespace camera {
        concept Camera = requires(
            auto         c,
            WindowBase*  w,
            f32          f,
            f32v2        f2,
            f32v3        f3,
            f32q         q
        ) {
            { c.update()                    } -> void

            { c.attach_to_window(w)         } -> void

            { c.aspect_ratio()              } -> f32
            { c.fov()                       } -> f32
            { c.near_clipping()             } -> f32
            { c.far_clipping()              } -> f32
            { c.clipping()                  } -> f32v2
            { c.position()                  } -> const f32v3&
            { c.direction()                 } -> const f32v3&
            { c.right()                     } -> const f32v3&
            { c.up()                        } -> const f32v3&
            { c.projection_matrix()         } -> const f32m4&
            { c.view_matrix()               } -> const f32m4&
            { c.view_projection_matrix()    } -> const f32m4&

            { c.set_aspect_ratio(f)         } -> void
            { c.set_fov(f)                  } -> void
            { c.set_near_clipping(f)        } -> void
            { c.set_far_clipping(f)         } -> void
            { c.set_clipping(f, f)          } -> void
            { c.set_position(f3)            } -> void
            { c.offset_position(f3)         } -> void

            { c.apply_rotation(q)           } -> void
            { c.rotate_from_mouse(f, f, f)  } -> void
            { c.roll_from_mouse(f, f)       } -> void
        }
    }
}
namespace hcam = hemlock::camera;

#endif // __hemlock_camera_camera_concept_hpp

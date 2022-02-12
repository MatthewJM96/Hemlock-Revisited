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
            f32v3        f3,
            f32q         q
        ) {
            { c.update()                    } -> void

            { c.attach_to_window(w)         } -> void

            { c.near_clipping()             } -> f32
            { c.far_clipping()              } -> f32
            { c.nf_clipping()               } -> f32v2
            { c.position()                  } -> const f32v3&
            { c.direction()                 } -> const f32v3&
            { c.right()                     } -> const f32v3&
            { c.up()                        } -> const f32v3&
            { c.projection_matrix()         } -> const f32m4&
            { c.view_matrix()               } -> const f32m4&
            { c.view_projection_matrix()    } -> const f32m4&

            { c.set_near_clipping(f)        } -> void
            { c.set_far_clipping(f)         } -> void
            { c.set_nf_clipping(f, f)       } -> void
            { c.set_position(f3)            } -> void
            { c.set_direction(f3)           } -> void
            { c.set_right(f3)               } -> void
            { c.set_up(f3)                  } -> void
            { c.offset_position(f3)         } -> void
        }

        concept OrthographicCamera = Camera && requires(
            auto c,
            f32  f
        ) {
            { c.left_clipping()             } -> f32
            { c.right_clipping()            } -> f32
            { c.lr_clipping()               } -> f32v2
            { c.down_clipping()             } -> f32
            { c.up_clipping()               } -> f32
            { c.du_clipping()               } -> f32v2

            { c.set_left_clipping(f)        } -> void
            { c.set_right_clipping(f)       } -> void
            { c.set_lr_clipping(f, f)       } -> void
            { c.set_down_clipping(f)        } -> void
            { c.set_up_clipping(f)          } -> void
            { c.set_du_clipping(f, f)       } -> void

            { c.set_all_clipping(f, f, f, f, f, f) } -> void
        }

        concept PerspectiveCamera = Camera && requires(
            auto c,
            f32  f
        ) {
            { c.aspect_ratio()              } -> f32
            { c.fov()                       } -> f32

            { c.set_aspect_ratio(f)         } -> void
            { c.set_fov(f)                  } -> void
        }
    }
}
namespace hcam = hemlock::camera;

#endif // __hemlock_camera_camera_concept_hpp

#ifndef __hemlock_camera_basic_orthographic_camera_h
#define __hemlock_camera_basic_orthographic_camera_h

#include "app/window/state.hpp"
#include "camera/base_camera.h"
#include "camera/state.hpp"

namespace hemlock {
    namespace app {
        class WindowBase;
    }

    namespace camera {
        class BasicOrthographicCamera : public BaseCamera<OrthographicCameraState> {
            BasicOrthographicCamera();

            virtual ~BasicOrthographicCamera() { /* Empty. */
            }

            void update();

            f32 left_clipping() { return m_state.left_clipping; }

            f32 right_clipping() { return m_state.right_clipping; }

            f32v2 lr_clipping() {
                return { m_state.left_clipping, m_state.right_clipping };
            }

            f32 down_clipping() { return m_state.down_clipping; }

            f32 up_clipping() { return m_state.up_clipping; }

            f32v2 du_clipping() {
                return { m_state.down_clipping, m_state.up_clipping };
            }

            void set_left_clipping(f32 left_clipping);
            void set_right_clipping(f32 right_clipping);
            void set_lr_clipping(f32 left_clipping, f32 right_clipping);
            void set_down_clipping(f32 down_clipping);
            void set_up_clipping(f32 up_clipping);
            void set_du_clipping(f32 down_clipping, f32 up_clipping);
            void set_all_clipping(
                f32 left_clipping,
                f32 right_clipping,
                f32 down_clipping,
                f32 up_clipping,
                f32 near_clipping,
                f32 far_clipping
            );

            // TODO(Matthew): How do we actually want to do this? We might want to,
            // e.g. lock to
            //                an axis, or wholly prevent free rotations in any axis.
            // TODO(Matthew): Would like to hook these up internally, but we don't
            // want to fix
            //                this kind of thing to mouse gestures, but optionally
            //                connect camera to controller inputs.
            void apply_rotation(f32q rotation);
            void rotate_from_mouse(f32 dx, f32 dy, f32 speed);
            void roll_from_mouse(f32 dx, f32 speed);
        };
    }  // namespace camera
}  // namespace hemlock
namespace hcam = hemlock::camera;

#endif  // __hemlock_camera_basic_orthographic_camera_h

#ifndef __hemlock_camera_basic_first_person_camera_h
#define __hemlock_camera_basic_first_person_camera_h

#include "app/window/state.hpp"
#include "camera/base_camera.h"
#include "camera/state.hpp"

namespace hemlock {
    namespace app {
        class WindowBase;
    }

    namespace camera {
        class BasicFirstPersonCamera : public BaseCamera<PerspectiveCameraState> {
        public:
            BasicFirstPersonCamera();

            virtual ~BasicFirstPersonCamera() { /* Empty. */
            }

            void update();

            f32 aspect_ratio() { return m_state.aspect_ratio; }

            f32 fov() { return m_state.fov; }

            bool clamp_up_enabled() { return m_clamp_up.enabled; }

            f32 clamp_up_angle() { return m_clamp_up.angle; }

            ClampAxis clamp_up() { return m_clamp_up; }

            void set_aspect_ratio(f32 aspect_ratio);
            void set_fov(f32 fov);
            void set_clamp_enabled(bool enabled);
            void set_clamp_angle(f32 angle);
            void set_clamp(ClampAxis clamp);

            // TODO(Matthew): Would like to hook these up internally, but we don't
            // want to fix
            //                this kind of thing to mouse gestures, but optionally
            //                connect camera to controller inputs.
            void apply_rotation_in_local_axes(f32q rotation);
            void apply_rotation_with_absolute_up(f32q rotation);
            void rotate_from_mouse_in_local_axes(f32 dx, f32 dy, f32 speed);
            void rotate_from_mouse_with_absolute_up(f32 dx, f32 dy, f32 speed);
            void roll_from_mouse(f32 dx, f32 speed);
        protected:
            void assure_clamp_up();

            ClampAxis m_clamp_up;
        };
    }  // namespace camera
}  // namespace hemlock
namespace hcam = hemlock::camera;

#endif  // __hemlock_camera_basic_first_person_camera_h

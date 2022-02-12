#ifndef __hemlock_camera_basic_first_person_camera_h
#define __hemlock_camera_basic_first_person_camera_h

#include "app/window/state.hpp"
#include "camera/state.hpp"
#include "camera/base_camera.h"

namespace hemlock {
    namespace app {
        class WindowBase;
    }

    namespace camera {
        class BasicFirstPersonCamera : public BaseCamera<PerspectiveCameraState> {
        public:
            BasicFirstPersonCamera();
            virtual ~BasicFirstPersonCamera() { /* Empty. */ }

            void update();

            f32 aspect_ratio() { return m_state.aspect_ratio; }
            f32 fov()          { return m_state.fov;          }

            void set_aspect_ratio(f32 aspect_ratio);
            void set_fov(f32 fov);

            // TODO(Matthew): Would like to hook these up internally, but we don't want to fix
            //                this kind of thing to mouse gestures, but optionally connect camera
            //                to controller inputs.
            void apply_rotation(f32q rotation);
            void rotate_from_mouse(f32 dx, f32 dy, f32 speed);
            void rotate_from_mouse_absolute_up(f32 dx, f32 dy, f32 speed, bool do_clamp = true, f32 clamp = 60.0f / 360.0f * 2.0f * M_PI);
            void roll_from_mouse(f32 dx, f32 speed);
        };
    }
}
namespace hcam = hemlock::camera;

#endif // __hemlock_camera_basic_first_person_camera_h

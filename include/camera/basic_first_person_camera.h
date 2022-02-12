#ifndef __hemlock_camera_basic_first_person_camera_h
#define __hemlock_camera_basic_first_person_camera_h

#include "app/window/state.hpp"
#include "camera/state.hpp"

namespace hemlock {
    namespace app {
        class WindowBase;
    }

    namespace camera {
        class BasicFirstPersonCamera {
            BasicFirstPersonCamera();
            ~BasicFirstPersonCamera() { /* Empty. */ }

            void update();

            void attach_to_window(app::WindowBase* window);

                  f32    aspect_ratio()             { return m_state.aspect_ratio;                              }
                  f32    fov()                      { return m_state.fov;                                       }
                  f32    near_clipping()            { return m_state.near_clipping;                             }
                  f32    far_clipping()             { return m_state.far_clipping;                              }
                  f32v2  clipping()                 { return { m_state.near_clipping, m_state.far_clipping };   }
            const f32v3& position()                 { return m_state.position;                                  }
            const f32v3& direction()                { return m_state.direction;                                 }
            const f32v3& right()                    { return m_state.right;                                     }
            const f32v3& up()                       { return m_state.up;                                        }
            const f32m4& projection_matrix()        { return m_state.projection_matrix;                         }
            const f32m4& view_matrix()              { return m_state.view_matrix;                               }
            const f32m4& view_projection_matrix()   { return m_state.view_projection_matrix;                    }

            void set_aspect_ratio(f32 aspect_ratio);
            void set_fov(f32 fov);
            void set_near_clipping(f32 near_clipping);
            void set_far_clipping(f32 far_clipping);
            void set_clipping(f32 near_clipping, f32 far_clipping);
            void set_position(f32v3 position);
            void offset_position(f32v3 delta_position);

            void apply_rotation(f32q rotation);
            void rotate_from_mouse(f32 dx, f32 dy, f32 speed);
            void roll_from_mouse(f32 dx, f32 speed);
        protected:
            Subscriber<happ::ResizeEvent> handle_window_resize;

            CommonCameraState m_state;
            
            bool m_view_changed;
            bool m_projection_changed;
        };
    }
}
namespace hcam = hemlock::camera;

#endif // __hemlock_camera_basic_first_person_camera_h

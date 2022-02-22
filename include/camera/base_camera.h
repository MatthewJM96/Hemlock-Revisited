#ifndef __hemlock_camera_base_camera_h
#define __hemlock_camera_base_camera_h

#include "app/window/state.hpp"
#include "camera/state.hpp"

namespace hemlock {
    namespace app {
        class WindowBase;
    }

    namespace camera {
        template <typename CameraState>
        class BaseCamera {
        public:
            BaseCamera(Subscriber<happ::ResizeEvent>&& handler) :
                handle_window_resize(std::forward<Subscriber<happ::ResizeEvent>>(handler)),
                m_view_changed(false),
                m_projection_changed(false)
            { /* Empty. */ };
            virtual ~BaseCamera() { /* Empty. */ }

            void attach_to_window(app::WindowBase* window);

                  f32    near_clipping()            { return m_state.near_clipping;                             }
                  f32    far_clipping()             { return m_state.far_clipping;                              }
                  f32v2  nf_clipping()              { return { m_state.near_clipping, m_state.far_clipping };   }
            const f32v3& position()                 { return m_state.position;                                  }
            const f32v3& direction()                { return m_state.direction;                                 }
            const f32v3& right()                    { return m_state.right;                                     }
            const f32v3& up()                       { return m_state.up;                                        }
            const f32m4& projection_matrix()        { return m_state.projection_matrix;                         }
            const f32m4& view_matrix()              { return m_state.view_matrix;                               }
            const f32m4& view_projection_matrix()   { return m_state.view_projection_matrix;                    }

            void set_near_clipping(f32 near_clipping);
            void set_far_clipping(f32 far_clipping);
            void set_nf_clipping(f32 near_clipping, f32 far_clipping);
            void set_position(f32v3 position);
            void offset_position(f32v3 delta_position);
        protected:
            Subscriber<happ::ResizeEvent> handle_window_resize;

            CameraState m_state;

            bool m_view_changed;
            bool m_projection_changed;
        };
    }
}
namespace hcam = hemlock::camera;

#include "base_camera.inl"

#endif // __hemlock_camera_base_camera_h

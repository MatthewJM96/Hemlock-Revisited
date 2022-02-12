#ifndef __hemlock_camera_state_hpp
#define __hemlock_camera_state_hpp

namespace hemlock {
    namespace camera {
        struct CommonCameraState {
            f32 aspect_ratio;
            f32 fov;
            f32 near_clipping;
            f32 far_clipping;

            f32v3 position;
            f32v3 direction;
            f32v3 right;
            f32v3 up;

            f32m4 view_matrix;
            f32m4 projection_matrix;
            f32m4 view_projection_matrix;
        };
    }
}
namespace hcam = hemlock::camera;

#endif // __hemlock_camera_state_hpp

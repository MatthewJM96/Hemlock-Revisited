#ifndef __hemlock_camera_state_hpp
#define __hemlock_camera_state_hpp

namespace hemlock {
    namespace camera {
        const f32v3 ABSOLUTE_UP = f32v3{0.0f, -1.0f, 0.0f};

        struct CommonCameraState {
            f32 near_clipping = 0.1f;
            f32 far_clipping  = 10000.0f;

            f32v3 position  = f32v3{0.0f};
            f32v3 direction = f32v3{0.0f, 0.0f, 1.0f};
            f32v3 right     = f32v3{1.0f, 0.0f, 0.0f};
            f32v3 up        = f32v3{0.0f, -1.0f, 0.0f};

            f32m4 view_matrix;
            f32m4 projection_matrix;
            f32m4 view_projection_matrix;
        };

        struct OrthographicCameraState : public CommonCameraState {
            f32 left_clipping;
            f32 right_clipping;
            f32 up_clipping;
            f32 down_clipping;
        };

        struct PerspectiveCameraState : public CommonCameraState {
            f32 aspect_ratio = 4.0f / 3.0f;
            f32 fov          = 60.0f;
        };
    }
}
namespace hcam = hemlock::camera;

#endif // __hemlock_camera_state_hpp

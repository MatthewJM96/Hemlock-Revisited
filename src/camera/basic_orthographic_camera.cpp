#include "stdafx.h"

#include "app/window/window_base.h"

#include "camera/basic_orthographic_camera.h"

hcam::BasicOrthographicCamera::BasicOrthographicCamera() :
    BaseCamera<OrthographicCameraState>(Subscriber<happ::ResizeEvent>([&](Sender, happ::ResizeEvent ev) {
        f32 dx = static_cast<f32>(ev.now.width)  / static_cast<f32>(ev.before.width);
        f32 dy = static_cast<f32>(ev.now.height) / static_cast<f32>(ev.before.height);

        if (dx != 1.0f) {
            f32 lr_com   = (m_state.left_clipping + m_state.right_clipping) / 2.0f;
            f32 lr_range = m_state.right_clipping - m_state.left_clipping;

            f32 new_lr_range_2 = (lr_range * dy) / 2.0f;

            set_lr_clipping(lr_com - new_lr_range_2, lr_com + new_lr_range_2);
        }

        if (dy != 1.0f) {
            f32 du_com   = (m_state.down_clipping + m_state.up_clipping) / 2.0f;
            f32 du_range = m_state.up_clipping - m_state.down_clipping;

            f32 new_du_range_2 = (du_range * dy) / 2.0f;

            set_du_clipping(du_com - new_du_range_2, du_com + new_du_range_2);
        }
    }))
{ /* Empty. */ }


void hcam::BasicOrthographicCamera::update() {
    if (m_view_changed)
        m_state.view_matrix = glm::lookAt(m_state.position, m_state.position + m_state.direction, m_state.up);

    if (m_projection_changed) {
        f32 lr_scale_component = 2.0f / (m_state.right_clipping - m_state.left_clipping);
        f32 du_scale_component = 2.0f / (m_state.up_clipping    - m_state.down_clipping);
        f32 nf_scale_component = 2.0f / (m_state.far_clipping   - m_state.near_clipping);

        f32 lr_translation_component = -1.0f * (m_state.right_clipping + m_state.left_clipping) / (m_state.right_clipping - m_state.left_clipping);
        f32 du_translation_component = -1.0f * (m_state.up_clipping    + m_state.down_clipping) / (m_state.up_clipping    - m_state.down_clipping);
        f32 nf_translation_component = -1.0f * (m_state.far_clipping   + m_state.near_clipping) / (m_state.far_clipping   - m_state.near_clipping);

        m_state.projection_matrix = f32m4{
            lr_scale_component,       0.0f,               0.0f,         lr_translation_component,
                  0.0f,         du_scale_component,       0.0f,         du_translation_component,
                  0.0f,               0.0f,         nf_scale_component, nf_translation_component,
                  0.0f,               0.0f,               0.0f,                 1.0f
        };
    }

    if (m_view_changed || m_projection_changed)
        m_state.view_projection_matrix = m_state.projection_matrix * m_state.view_matrix;

    m_view_changed       = false;
    m_projection_changed = false;
}

void hcam::BasicOrthographicCamera::set_left_clipping(f32 left_clipping) {
    m_state.left_clipping = left_clipping;
    m_projection_changed  = true;
}

void hcam::BasicOrthographicCamera::set_right_clipping(f32 right_clipping) {
    m_state.right_clipping = right_clipping;
    m_projection_changed  = true;
}

void hcam::BasicOrthographicCamera::set_lr_clipping(f32 left_clipping, f32 right_clipping) {
    m_state.left_clipping  = left_clipping;
    m_state.right_clipping = right_clipping;
    m_projection_changed   = true;
}

void hcam::BasicOrthographicCamera::set_down_clipping(f32 down_clipping) {
    m_state.down_clipping = down_clipping;
    m_projection_changed  = true;
}

void hcam::BasicOrthographicCamera::set_up_clipping(f32 up_clipping) {
    m_state.up_clipping  = up_clipping;
    m_projection_changed = true;
}

void hcam::BasicOrthographicCamera::set_du_clipping(f32 down_clipping, f32 up_clipping) {
    m_state.down_clipping = down_clipping;
    m_state.up_clipping   = up_clipping;
    m_projection_changed  = true;
}

void hcam::BasicOrthographicCamera::set_all_clipping(
    f32 left_clipping,
    f32 right_clipping,
    f32 down_clipping,
    f32 up_clipping,
    f32 near_clipping,
    f32 far_clipping
) {
    m_state.left_clipping  = left_clipping;
    m_state.right_clipping = right_clipping;
    m_state.down_clipping  = down_clipping;
    m_state.up_clipping    = up_clipping;
    m_state.near_clipping  = near_clipping;
    m_state.far_clipping   = far_clipping;
    m_projection_changed   = true;
}

void hcam::BasicOrthographicCamera::apply_rotation(f32q rotation) {
    m_state.direction = glm::normalize(rotation * m_state.direction);
    m_state.right     = glm::normalize(rotation * m_state.right);
    m_state.up        = glm::normalize(glm::cross(m_state.right, m_state.direction));

    m_view_changed = true;
}

void hcam::BasicOrthographicCamera::rotate_from_mouse(f32 dx, f32 dy, f32 speed) {
    f32q up    = glm::angleAxis(dy * speed, m_state.right);
    f32q right = glm::angleAxis(dx * speed, m_state.up);

    apply_rotation(up * right);
}
void hcam::BasicOrthographicCamera::roll_from_mouse(f32 dx, f32 speed) {
    f32q forward = glm::angleAxis(dx * speed, m_state.direction);

    apply_rotation(forward);
}

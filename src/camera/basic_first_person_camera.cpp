#include "stdafx.h"

#include "app/window/window_base.h"

#include "camera/basic_first_person_camera.h"

hcam::BasicFirstPersonCamera::BasicFirstPersonCamera() :
    BaseCamera<PerspectiveCameraState>(Subscriber<happ::ResizeEvent>{[&](Sender, happ::ResizeEvent ev) {
        f32 new_aspect_ratio = static_cast<f32>(ev.now.width) / static_cast<f32>(ev.now.height);
        if (m_state.aspect_ratio != new_aspect_ratio)
            set_aspect_ratio(new_aspect_ratio);
    }}),
    m_clamp_up({true, 60.0f / 360.0f * 2.0f * static_cast<f32>(M_PI)})
{ /* Empty. */ }

void hcam::BasicFirstPersonCamera::update() {
    if (m_view_changed)
        m_state.view_matrix = glm::lookAt(m_state.position, m_state.position + m_state.direction, m_state.up);

    if (m_projection_changed)
        m_state.projection_matrix = glm::perspective(m_state.fov, m_state.aspect_ratio, m_state.near_clipping, m_state.far_clipping);

    if (m_view_changed || m_projection_changed)
        m_state.view_projection_matrix = m_state.projection_matrix * m_state.view_matrix;

    m_view_changed       = false;
    m_projection_changed = false;
}

void hcam::BasicFirstPersonCamera::set_aspect_ratio(f32 aspect_ratio) {
    m_state.aspect_ratio = aspect_ratio;
    m_projection_changed = true;
}

void hcam::BasicFirstPersonCamera::set_fov(f32 fov) {
    m_state.fov          = fov;
    m_projection_changed = true;
}

void hcam::BasicFirstPersonCamera::set_clamp_enabled(bool enabled) {
    m_clamp_up.enabled = enabled;
    if (enabled) assure_clamp_up();
}

void hcam::BasicFirstPersonCamera::set_clamp_angle(f32 angle) {
    m_clamp_up.angle = angle;
    if (m_clamp_up.enabled) assure_clamp_up();
}

void hcam::BasicFirstPersonCamera::set_clamp(ClampAxis clamp) {
    m_clamp_up = clamp;
    if (m_clamp_up.enabled) assure_clamp_up();
}

void hcam::BasicFirstPersonCamera::apply_rotation_in_local_axes(f32q rotation) {
    m_state.direction = glm::normalize(rotation * m_state.direction);
    m_state.right     = glm::normalize(rotation * m_state.right);
    m_state.up        = glm::normalize(glm::cross(m_state.right, m_state.direction));

    m_view_changed = true;
}

void hcam::BasicFirstPersonCamera::apply_rotation_with_absolute_up(f32q rotation) {
    m_state.direction = glm::normalize(rotation * m_state.direction);
    m_state.right     = glm::normalize(glm::cross(m_state.direction, ABSOLUTE_UP));
    m_state.up        = glm::normalize(glm::cross(m_state.right, m_state.direction));

    m_view_changed = true;
}

void hcam::BasicFirstPersonCamera::rotate_from_mouse_in_local_axes(f32 dx, f32 dy, f32 speed) {
    f32q up    = glm::angleAxis(dy * speed, m_state.right);
    f32q right = glm::angleAxis(dx * speed, m_state.up);

    apply_rotation_in_local_axes(up * right);
}

void hcam::BasicFirstPersonCamera::rotate_from_mouse_with_absolute_up(f32 dx, f32 dy, f32 speed) {
    f32q up    = glm::angleAxis(dy * speed, m_state.right);
    f32q right = glm::angleAxis(dx * speed, ABSOLUTE_UP);

    struct {
        f32v3 direction, up, right;
    } previous = {
        m_state.direction, m_state.up, m_state.right
    };

    apply_rotation_with_absolute_up(up * right);

    f32 up_angle = glm::acos(glm::dot(m_state.up, ABSOLUTE_UP));
    if (m_clamp_up.enabled && (-1.0f * m_clamp_up.angle > up_angle || m_clamp_up.angle < up_angle)) {
        m_state.direction = previous.direction;
        m_state.up        = previous.up;
        m_state.right     = previous.right;

        m_clamp_up.enabled = false;
        rotate_from_mouse_with_absolute_up(dx, 0.0f, speed);
        m_clamp_up.enabled = true;
    }
}

void hcam::BasicFirstPersonCamera::roll_from_mouse(f32 dx, f32 speed) {
    f32q forward = glm::angleAxis(dx * speed, m_state.direction);

    apply_rotation_in_local_axes(forward);
}

void hcam::BasicFirstPersonCamera::assure_clamp_up() {
    f32 up_angle = glm::acos(glm::dot(m_state.up, ABSOLUTE_UP));

    f32 correction_angle = 0.0;
    if (up_angle < -1.0f * m_clamp_up.angle) {
        correction_angle = -1.0f * m_clamp_up.angle - up_angle;
    } else if (up_angle > m_clamp_up.angle) {
        correction_angle = m_clamp_up.angle - up_angle;
    }
    f32q correction = glm::angleAxis(correction_angle, m_state.right);
    apply_rotation_with_absolute_up(correction);
}

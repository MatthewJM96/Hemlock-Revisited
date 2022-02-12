#include "stdafx.h"

#include "app/window/window_base.h"

#include "camera/basic_first_person_camera.h"

hcam::BasicFirstPersonCamera::BasicFirstPersonCamera() :
    BaseCamera<PerspectiveCameraState>(Subscriber<happ::ResizeEvent>([&](Sender, happ::ResizeEvent ev) {
        f32 new_aspect_ratio = static_cast<f32>(ev.now.width) / static_cast<f32>(ev.now.height);
        if (m_state.aspect_ratio != new_aspect_ratio)
            set_aspect_ratio(new_aspect_ratio);
    }))
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

void hcam::BasicFirstPersonCamera::apply_rotation(f32q rotation) {
    m_state.direction = glm::normalize(rotation * m_state.direction);
    m_state.right     = glm::normalize(rotation * m_state.right);
    m_state.up        = glm::normalize(glm::cross(m_state.right, m_state.direction));

    m_view_changed = true;
}

void hcam::BasicFirstPersonCamera::rotate_from_mouse(f32 dx, f32 dy, f32 speed) {
    f32q up    = glm::angleAxis(dy * speed, m_state.right);
    f32q right = glm::angleAxis(dx * speed, m_state.up);

    apply_rotation(up * right);
}
void hcam::BasicFirstPersonCamera::roll_from_mouse(f32 dx, f32 speed) {
    f32q forward = glm::angleAxis(dx * speed, m_state.direction);

    apply_rotation(forward);
}

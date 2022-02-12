
template <typename CameraState>
void hcam::BaseCamera<CameraState>::attach_to_window(app::WindowBase* window) {
    window->on_window_resize += &handle_window_resize;
}

template <typename CameraState>
void hcam::BaseCamera<CameraState>::set_near_clipping(f32 near_clipping) {
    m_state.near_clipping = near_clipping;
    m_projection_changed  = true;
}

template <typename CameraState>
void hcam::BaseCamera<CameraState>::set_far_clipping(f32 far_clipping) {
    m_state.far_clipping = far_clipping;
    m_projection_changed = true;
}

template <typename CameraState>
void hcam::BaseCamera<CameraState>::set_nf_clipping(f32 near_clipping, f32 far_clipping) {
    m_state.near_clipping = near_clipping;
    m_state.far_clipping  = far_clipping;
    m_projection_changed  = true;
}

template <typename CameraState>
void hcam::BaseCamera<CameraState>::set_position(f32v3 position) {
    m_state.position = position;
    m_view_changed   = true;
}

template <typename CameraState>
void hcam::BaseCamera<CameraState>::offset_position(f32v3 delta_position) {
    m_state.position += delta_position;
    m_view_changed    = true;
}

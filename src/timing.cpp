#include "stdafx.h"

#include "timing.h"

#define MILLISECONDS 1000.0

void hemlock::FpsTimer::begin() {
    m_start_time = SDL_GetTicks();
}

f64 hemlock::FpsTimer::end() {
    calculate_fps();

    return m_fps;
}

// TODO(Matthew): At some point compare with a sampling method that looks at previous N frame times.
void hemlock::FpsTimer::calculate_fps() {
    static const f64 smoothing_factor = 0.9;

    f64 time_this_frame = static_cast<f64>(SDL_GetTicks() - m_start_time);

    m_frame_time = (m_frame_time * smoothing_factor) + time_this_frame * (1.0 - smoothing_factor);
    
    if (m_frame_time == 0.0) {
        m_fps = 0.0;
    } else {
        m_fps = static_cast<f64>(MILLISECONDS / m_frame_time);
    }
}

// TODO(Matthew): We want this to optionally (default?) only be on rendering.
//                  no point wasting CPU cycles!
f64 hemlock::FpsLimiter::end() {
    calculate_fps();

    f64 time_this_frame = static_cast<f64>(SDL_GetTicks() - m_start_time);
    if (MILLISECONDS / m_max_fps > time_this_frame) {
        // TODO(Matthew): This won't actually achieve anything like what we want unless we're targetting <30 FPS.
        //                  we need to use a higher resolution mechanism.
        SDL_Delay(static_cast<ui32>((MILLISECONDS / m_max_fps) - time_this_frame));
    }

    return m_fps;
}

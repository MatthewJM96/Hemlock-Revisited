#include "stdafx.h"

#include "timing.h"

#define NANOSECONDS 1000000000.0f

void hemlock::FrameTimer::start() {
    m_last_frame_point = std::chrono::steady_clock::now();
}

void hemlock::FrameTimer::frame_end() {
    auto now = std::chrono::steady_clock::now();
    m_frame_times.push_back(now - m_last_frame_point);
    m_last_frame_point = now;
}

f32 hemlock::FrameTimer::fps() {
    ui64 total_time = 0;
    for (FrameTime& frame_time : m_frame_times) {
        total_time += frame_time.count();
    }

    return NANOSECONDS / static_cast<f32>(total_time);
}

void hemlock::FrameLimiter::frame_end() {
    FrameTimer::frame_end();

    if (m_target_fps <= 0.0f) return;

    while ((std::chrono::steady_clock::now() - m_last_frame_point).count()
           < static_cast<i64>(NANOSECONDS / m_target_fps))
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

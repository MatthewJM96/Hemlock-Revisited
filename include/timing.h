#ifndef __hemlock_timing_h
#define __hemlock_timing_h

namespace hemlock {
    using FramePoint = std::chrono::steady_clock::time_point;
    using FrameTime  = std::chrono::nanoseconds;
    using FrameTimes = boost::circular_buffer<FrameTime>;

    template <typename Units = std::chrono::milliseconds, typename Precision = f32>
    Precision frame_time_to_floating(FrameTime frame_time) {
        return static_cast<Precision>(frame_time.count()) / static_cast<Precision>(Units::period::den);
    }

    class FrameTimer {
    public:
        FrameTimer(size_t frames_count = 5) :
            m_frame_times(FrameTimes(frames_count))
        { /* Empty. */ }
        virtual ~FrameTimer() { /* Empty. */ }

        virtual void start();
        virtual void frame_end();

        const FrameTimes& frame_times()         { return m_frame_times;         }
                FrameTime latest_frame_time()   { return m_frame_times.back();  }

        f32 fps();
    protected:
        FramePoint  m_last_frame_point;
        FrameTimes  m_frame_times;
    };

    class FrameLimiter : public FrameTimer {
    public:
        FrameLimiter(size_t frames_count, f32 target_fps) :
            FrameTimer(frames_count), m_target_fps(target_fps)
        { /* Empty. */ }
        virtual ~FrameLimiter() { /* Empty. */ }

        f32 target_fps() { return m_target_fps; }

        void set_target_fps(f32 target_fps) { m_target_fps = target_fps; }

        virtual void frame_end() override;
    private:
        f32 m_target_fps;
    };
}

#endif // __hemlock_timing_h

#ifndef __hemlock_timing_h
#define __hemlock_timing_h

namespace hemlock {
    class FpsTimer {
    public:
        FpsTimer() { /* Empty. */ }
        virtual ~FpsTimer() { /* Empty. */ }

        virtual void begin();
        virtual f64 end();

        f64 frame_time() const { return m_frame_time; }
        f64 fps()        const { return m_fps; }
    protected:
        f64  m_frame_time = 0.0;
        f64  m_fps        = 0.0;
        ui32 m_start_time =   0;

        void calculate_fps();
    };

    class FpsLimiter : public FpsTimer {
    public:
        FpsLimiter() { /* Empty. */ }
        virtual ~FpsLimiter() { /* Empty. */ }

        void init(f64 max_fps) { m_max_fps = max_fps; }

        void set_max_fps(f64 max_fps) { m_max_fps = max_fps; }

        f64 end();
    private:
        f64 m_max_fps;
    };
}

#endif // __hemlock_timing_h

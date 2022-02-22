#ifndef __hemlock_app_screen_h
#define __hemlock_app_screen_h

namespace hemlock {
    namespace app {
        class ProcessBase;
        class ScreenBase;

        enum class ScreenState {
            NONE,            // Screen is doing nothing.
            RUNNING,         // Screen is running and being drawn.
            CHANGE_NEXT,     // Screen wants application to move onto next screen.
            CHANGE_PREV,     // Screen wants application to move back to last screen.
            QUIT_APPLICATION // Screen wants application to quit.
        };

        /**
         * @brief A screen provides an interface for a set of update and rendering logic.
         */
        class ScreenBase {
        public:
            ScreenBase() :
                m_initialised(false),
                m_name(""),
                m_state(ScreenState::NONE),
                m_process(nullptr),
                m_next_screen(nullptr),
                m_prev_screen(nullptr)
            { /* Empty. */ };
            virtual ~ScreenBase() { /* Empty */};

            virtual void init(const std::string& name, ProcessBase* process);
            virtual void dispose(); 

            virtual void start (TimeData time);
            virtual void end   (TimeData time);

            virtual void update (TimeData time) = 0;
            virtual void draw   (TimeData time) = 0;

            bool is_initialised() const { return m_initialised; }
            bool is_running()     const { return m_state == ScreenState::RUNNING; }

            const std::string& name()  const { return m_name;  }
                   ScreenState state() const { return m_state; }

            ScreenBase* next_screen() const { return m_next_screen; }
            ScreenBase* prev_screen() const { return m_prev_screen; }

            void set_next_screen(ScreenBase* screen) { m_next_screen = screen; }
            void set_prev_screen(ScreenBase* screen) { m_prev_screen = screen; }
        protected:
            bool m_initialised;

            std::string m_name;
            ScreenState m_state;

            ProcessBase* m_process;

            ScreenBase* m_next_screen;
            ScreenBase* m_prev_screen;
        };
    }
}
namespace happ = hemlock::app;

#endif // __hemlock_app_screen_h

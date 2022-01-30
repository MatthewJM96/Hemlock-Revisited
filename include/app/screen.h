#ifndef __hemlock_app_screen_h
#define __hemlock_app_screen_h

namespace hemlock {
    namespace app {
        // TODO(Matthew): Allow screen to pass some amount of information to next/previous screen.
        enum class ScreenState {
            NONE,            // Screen is doing nothing.
            RUNNING,         // Screen is running and being drawn.
            CHANGE_NEXT,     // Screen wants application to move onto next screen.
            CHANGE_PREV,     // Screen wants application to move back to last screen.
            QUIT_APPLICATION // Screen wants application to quit.
        };

        // Not just for game screens, can also be used for UI screens with navigation back and forth.
        class IScreen {
        public:
            IScreen() :
                m_initialised(false),
                m_name(""),
                m_next_screen(nullptr),
                m_prev_screen(nullptr)
            { /* Empty. */ };
            virtual ~IScreen() { /* Empty */};

            virtual void init(std::string name);
            virtual void dispose(); 

            virtual void start(TimeData time);
            virtual void end(TimeData time);

            virtual void update(TimeData time) = 0;
            virtual void draw(TimeData time)   = 0;

            bool is_initialised() const { return m_initialised; }
            bool is_running()     const { return m_state == ScreenState::RUNNING; }

            const std::string& name() const { return m_name;  }

            ScreenState state() const { return m_state; }

            IScreen* next_screen() const { return m_next_screen; }
            IScreen* prev_screen() const { return m_prev_screen; }

            void set_next_screen(IScreen* screen) { m_next_screen = screen; }
            void set_prev_screen(IScreen* screen) { m_prev_screen = screen; }
        protected:
            bool        m_initialised;
            ScreenState m_state;
            std::string m_name;

            IScreen* m_next_screen;
            IScreen* m_prev_screen;
        };
    }
}
namespace happ = hemlock::app;

#endif // __hemlock_app_screen_h

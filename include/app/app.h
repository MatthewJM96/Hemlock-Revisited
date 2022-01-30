#ifndef __hemlock_app_app_h
#define __hemlock_app_app_h

namespace hemlock {
    class FpsLimiter;
    namespace graphics {
        class WindowManager;
    }
    namespace ui {
        class InputManager;
    }

    namespace app {
        class IScreen;

        struct ScreenChangeEvent {
            IScreen *before, *now;
        };

        class IApp {
        protected:
            using ScreenList = std::unordered_map<std::string, IScreen*>;
            using Screen     = std::pair<std::string, IScreen*>;
        public:
            IApp() : m_current_screen(nullptr) { /* Empty */ };
            virtual ~IApp() { /* Empty */ };

            virtual void init()    = 0;
            virtual void dispose() = 0;

            virtual void run() = 0;

            bool change_screen(std::string name);

            virtual void quit();

            Event<ScreenChangeEvent> on_screen_change;
        protected:      
            virtual void prepare_screens() = 0;

            bool add_screen(Screen screen);

            IScreen* current_screen() { return m_current_screen; }

            bool handle_requests();

            void goto_next_screen();
            void goto_prev_screen();

            void dispose_screens();

            bool m_initialised = false;

            TimeData m_current_times;
            TimeData m_previous_times;

            bool m_should_quit = false;

            ScreenList m_screens;
            IScreen*   m_current_screen;
        };

        class BasicApp : public IApp {
        public:
            BasicApp() :
                IApp(),
                m_window_manager(nullptr),
                m_input_manager(nullptr),
                m_fps_limiter(nullptr)
            { /* Empty */ };
            virtual ~BasicApp() { /* Empty */ };

            hemlock::graphics::WindowManager* window_manager() const { return m_window_manager; }
                   hemlock::ui::InputManager* input_manager()  const { return m_input_manager;  }

            virtual void init()    override;
            virtual void dispose() override;

            virtual void run() override;
        protected:
            void calculate_times();

            hemlock::graphics::WindowManager* m_window_manager;
            hemlock::ui::InputManager*        m_input_manager;
            hemlock::FpsLimiter*              m_fps_limiter;
        };
    }
}
namespace happ = hemlock::app;

#endif // __hemlock_app_app_h

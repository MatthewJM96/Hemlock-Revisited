#ifndef __hemlock_app_app_h
#define __hemlock_app_app_h

namespace hemlock {
    class FpsLimiter;
    namespace graphics {
        class WindowManagerBase;
    }
    namespace ui {
        class InputManager;
    }

    namespace app {
        class ScreenBase;

        struct ScreenChangeEvent {
            ScreenBase *before, *now;
        };

        class AppBase {
        protected:
            using ScreenList = std::unordered_map<std::string, ScreenBase*>;
            using Screen     = std::pair<std::string, ScreenBase*>;
        public:
            AppBase();
            virtual ~AppBase() { /* Empty */ };

            virtual void init()    = 0;
            virtual void dispose() = 0;

            virtual void run() = 0;

            bool should_quit() { return m_should_quit; }
            void set_should_quit(bool should_quit = true);

            bool change_screen(std::string name);

            hemlock::graphics::WindowManagerBase* window_manager() const { return m_window_manager; }

            Event<>                  on_quit;
            Event<ScreenChangeEvent> on_screen_change;
        protected:
            Subscriber<> handle_external_quit;

            virtual void prepare_screens() = 0;

            bool add_screen(Screen screen);

            ScreenBase* current_screen() { return m_current_screen; }

            virtual void quit();

            /**
             * @brief Handles requests for screen changes based on
             * current screen state.
             *
             * @return true if the screen is ready to be acted on, false otherwise.
             */
            bool handle_screen_requests();

            void goto_next_screen();
            void goto_prev_screen();

            void dispose_screens();

            bool m_initialised = false;

            TimeData m_current_times;
            TimeData m_previous_times;

            bool m_should_quit = false;

            ScreenList  m_screens;
            ScreenBase* m_current_screen;

            hemlock::graphics::WindowManagerBase* m_window_manager;
        };

        class BasicApp : public AppBase {
        public:
            BasicApp() :
                AppBase(),
                m_input_manager(nullptr),
                m_fps_limiter(nullptr)
            { /* Empty */ };
            virtual ~BasicApp() { /* Empty */ };

            hemlock::ui::InputManager* input_manager()  const { return m_input_manager;  }

            virtual void init()    override;
            virtual void dispose() override;

            virtual void run() override;
        protected:
            void calculate_times();

            hemlock::ui::InputManager*        m_input_manager;
            hemlock::FpsLimiter*              m_fps_limiter;
        };
    }
}
namespace happ = hemlock::app;

#endif // __hemlock_app_app_h

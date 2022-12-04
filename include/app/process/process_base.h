#ifndef __hemlock_app_process_process_h
#define __hemlock_app_process_process_h

namespace hemlock {
    namespace app {
        class ScreenBase;
        class WindowBase;

        using Screen  = std::pair<std::string, ScreenBase*>;
        using Screens = std::unordered_map<std::string, ScreenBase*>;

        struct ScreenChangeEvent {
            ScreenBase *before, *now;
        };

        class ProcessBase {
        public:
            ProcessBase();

            virtual ~ProcessBase() { /* Empty. */
            }

            virtual void init();
            virtual void dispose();

            /**
             * @brief Implementation of this depends on if it is
             * expecting to run in a multi-threaded or single-threaded
             * situation when joined by other processes.
             */
            virtual void run() = 0;

            bool add_screen(Screen&& screen);

            bool go_to_screen(const std::string& name, FrameTime time);

            WindowBase* window() { return m_window; }

            ScreenBase* current_screen() { return m_current_screen; }

            FrameTimer* timer() { return m_timer; }

            bool should_end_process() { return m_should_end_process; }

            void set_should_end_process(bool should_end_process = true);

            Event<>                  on_process_end;
            Event<ScreenChangeEvent> on_screen_change;
        protected:
            virtual void prepare_window()  = 0;
            virtual void prepare_screens() = 0;

            /**
             * @brief Handles requests for screen changes based on
             * current screen state.
             *
             * @return True if the screen is ready to be acted on, false otherwise.
             */
            bool handle_screen_requests();

            void goto_next_screen();
            void goto_prev_screen();

            void dispose_screens();

            virtual void end_process() = 0;

            bool m_initialised;

            volatile bool m_should_end_process;

            Screens     m_screens;
            ScreenBase* m_current_screen;

            WindowBase* m_window;
            FrameTimer* m_timer;
        };
    }  // namespace app
}  // namespace hemlock
namespace happ = hemlock::app;

#endif  // __hemlock_app_process_process_h

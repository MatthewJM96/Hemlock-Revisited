#ifndef __hemlock_app_single_window_app_h
#define __hemlock_app_single_window_app_h

#include "app/app_base.h"

namespace hemlock {
    class FpsLimiter;
    namespace ui {
        class InputManager;
    }

    namespace app {
        class ScreenBase;

        class SingleWindowApp : public AppBase {
        public:
            SingleWindowApp() :
                AppBase(),
                m_input_manager(nullptr),
                m_fps_limiter(nullptr)
            { /* Empty */ };
            virtual ~SingleWindowApp() { /* Empty */ };

            virtual void init()    override;
            virtual void dispose() override;

            virtual void run() override;

            bool change_screen(std::string name);

            hemlock::ui::InputManager* input_manager() const { return m_input_manager; }

            Event<ScreenChangeEvent> on_screen_change;
        protected:
            virtual void prepare_screens() = 0;

            bool add_screen(Screen screen);

            ScreenBase* current_screen() { return m_current_screen; }

            /**
             * @brief Handles requests for screen changes based on
             * current screen state.
             *
             * @return true if the screen is ready to be acted on, false otherwise.
             */
            bool handle_screen_requests();

            void goto_next_screen();
            void goto_prev_screen();
            
            void calculate_times();

            void dispose_screens();

            TimeData m_current_times;
            TimeData m_previous_times;

            ScreenList  m_screens;
            ScreenBase* m_current_screen;

            hemlock::ui::InputManager* m_input_manager;
            hemlock::FpsLimiter*       m_fps_limiter;
        };
    }
}
namespace happ = hemlock::app;

#endif // __hemlock_app_single_window_app_h

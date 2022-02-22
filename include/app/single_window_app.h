#ifndef __hemlock_app_single_window_app_h
#define __hemlock_app_single_window_app_h

#include "app/app_base.h"
#include "app/process/process_base.h"

namespace hemlock {
    class FpsLimiter;
    namespace ui {
        class InputManager;
    }

    namespace app {
        class ScreenBase;

        class SingleWindowApp : public AppBase, public ProcessBase {
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

            hemlock::ui::InputManager* input_manager() const { return m_input_manager; }
        protected:
            virtual void prepare_window() override;

            void calculate_times();

            virtual void end_process() override { dispose(); }

            hemlock::ui::InputManager* m_input_manager;
            hemlock::FpsLimiter*       m_fps_limiter;
        };
    }
}
namespace happ = hemlock::app;

#endif // __hemlock_app_single_window_app_h

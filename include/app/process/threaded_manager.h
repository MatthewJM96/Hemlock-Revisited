#ifndef __hemlock_app_process_threaded_manager_h
#define __hemlock_app_process_threaded_manager_h

#include "app/window/state.hpp"
#include "app/window/window.h"
#include "ui/input/dispatcher.h"

namespace hemlock {
    namespace ui {
        class InputDispatcher;
    }
    namespace app {
        class AppBase;

        using Windows = std::unordered_map<ui32, Window*>;

        class WindowManagerBase {
        public:
            WindowManagerBase() :
                m_main_window(nullptr),
                m_app(nullptr)
            { /* Empty. */ };
            virtual ~WindowManagerBase() { /* Empty. */ }

            virtual WindowError init(AppBase* app) = 0;
            virtual void dispose() = 0;

            virtual void sync_windows() = 0;

            Window* main_window() { return m_main_window; }
        protected:
            Window* m_main_window;

            AppBase* m_app;
        };

        class WindowManager : public WindowManagerBase {
        public:
            WindowManager();
            virtual ~WindowManager() { /* Empty. */ }

            virtual WindowError init(AppBase* app) override;
            virtual void dispose() override;

            void set_quit_on_main_window_close(bool should = true) { m_quit_on_main_window_close = should; }

            bool set_main_window(Window* window);
            bool set_main_window(ui32 window_id);

            std::pair<Window*, WindowError>
                add_window(WindowSettings settings = {});

            bool add_window(CALLEE_DELETE Window* window);

            bool dispose_window(Window* window);
            bool dispose_window(ui32 window_id);

            virtual void sync_windows() override;
        private:
            Subscriber<hui::WindowEvent> handle_window_close;

            Windows m_windows;

            bool m_quit_on_main_window_close;
        };
    }
}
namespace happ = hemlock::app;

#endif // __hemlock_app_process_threaded_manager_h

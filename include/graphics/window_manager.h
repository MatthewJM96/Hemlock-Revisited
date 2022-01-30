#ifndef __hemlock_graphics_window_manager_h
#define __hemlock_graphics_window_manager_h

#include "graphics/window.h"
#include "ui/input/dispatcher.h"

namespace hemlock {
    namespace app {
        class IApp;
    }
    namespace ui {
        class InputDispatcher;
    }

    namespace graphics {
        using Windows = std::unordered_map<ui32, Window*>;

        class WindowManager {
        public:
            WindowManager();
            ~WindowManager() { /* Empty. */ }

            WindowError init(hemlock::app::IApp* app);
            void dispose();

            void set_quit_on_main_window_close(bool should = true) { m_quit_on_main_window_close = should; }

            bool set_main_window(Window* window);
            bool set_main_window(ui32 window_id);

            std::pair<Window*, WindowError>
                add_window(WindowSettings settings = {});

            bool add_window(CALLEE_DELETE Window* window);

            bool dispose_window(Window* window);
            bool dispose_window(ui32 window_id);

            void sync_windows();
        private:
            Subscriber<hui::WindowEvent> handle_window_close;

            Windows m_windows;
            Window* m_main_window;

            hemlock::app::IApp* m_app;

            bool m_quit_on_main_window_close;
        };
    }
}
namespace hg = hemlock::graphics;

#endif // __hemlock_graphics_window_manager_h

#ifndef __hemlock_ui_input_dispatcher_h
#define __hemlock_ui_input_dispatcher_h

#include "ui/input/events.hpp"
#include "ui/input/keys.hpp"

namespace hemlock {
    namespace ui {
        class InputManager;

        class InputDispatcher {
        public:
            static InputDispatcher* instance() {
                if (m_instance == nullptr) {
                    m_instance = new InputDispatcher();
                }
                return m_instance;
            }

            InputDispatcher(const InputDispatcher&) = delete;
            void operator=(const InputDispatcher&)  = delete;
            ~InputDispatcher(){ /* Empty. */ };

            void init(InputManager* manager);
            void dispose();

            void set_text_mode(bool on);

            static i32 handle_event(void* data, SDL_Event* event);

            MouseEvents    on_mouse;
            KeyboardEvents on_keyboard;
            WindowEvents   on_window;
            TextEvents     on_text;
            DropEvents     on_drop;
            Event<>        on_quit;
        private:
            InputDispatcher() :
                m_initialised(false), m_manager(nullptr){ /* Empty. */ };

            static InputDispatcher* m_instance;

            bool m_initialised;

            InputManager* m_manager;
        };
    }  // namespace ui
}  // namespace hemlock
namespace hui = hemlock::ui;

#endif  // __hemlock_ui_input_dispatcher_h

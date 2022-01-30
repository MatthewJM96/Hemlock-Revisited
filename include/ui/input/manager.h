#ifndef __hemlock_ui_input_manager_h
#define __hemlock_ui_input_manager_h

#include "ui/input/keys.hpp"
#include "ui/input/state.hpp"

namespace hemlock {
    namespace ui {
        using KeyboardKeyStates = std::unordered_map<PhysicalKey, ButtonState>;

        class InputManager {
        public:
            InputManager()  { /* Empty. */ }
            ~InputManager() { /* Empty. */ }

            void init();
            void dispose();

            MouseCoords      mouse_coords()                      { return m_mouse_coords;               }
            ButtonState      mouse_button_state(ui8 button)      { return m_mouse_button_state[button]; }
            KeyModifiers     key_modifier_state()                { return m_key_modifier_state;         }
            CommonMouseState common_mouse_state();
            ButtonState      keyboard_key_state(PhysicalKey key);

            void set_mouse_coords(MouseCoords coords) { m_mouse_coords = coords; }

            void set_mouse_focus(bool focus)    { m_mouse_focus = focus;    }
            void set_keyboard_focus(bool focus) { m_keyboard_focus = focus; }

            bool is_mouse_focused()    { return m_mouse_focus;    }
            bool is_keyboard_focused() { return m_keyboard_focus; }

            bool is_pressed(ui8 button) { return m_mouse_button_state[button].is_pressed; }
            bool is_pressed(PhysicalKey key);

            void press(ui8 button);
            void release(ui8 button);

            void press(PhysicalKey key);
            void release(PhysicalKey key);
        private:
            i32 m_mouse_focus;
            i32 m_keyboard_focus;

            MouseCoords       m_mouse_coords;
            ButtonState       m_mouse_button_state[255];
            KeyboardKeyStates m_keyboard_key_state;
            KeyModifiers      m_key_modifier_state;
        };
    }
}
namespace hui = hemlock::ui;

#endif // __hemlock_ui_input_manager_h

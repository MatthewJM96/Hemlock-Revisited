#ifndef __hemlock_ui_input_state_hpp
#define __hemlock_ui_input_state_hpp

namespace hemlock {
    namespace ui {
        enum class MouseButton : ui8 {
            LEFT = 0,
            RIGHT,
            MIDDLE,
            UNKNOWN
        };

        struct MouseCoords {
            i32 x, y;
        };

        struct CommonMouseState {
            bool left;
            bool right;
            bool middle;
        };

        struct KeyModifiers {
            union {
                struct {
                    bool lshift : 1;
                    bool rshift : 1;
                    bool lctrl  : 1;
                    bool rctrl  : 1;
                    bool lalt   : 1;
                    bool ralt   : 1;
                    bool lcmd   : 1;
                    bool rcmd   : 1;
                };

                struct {
                    bool shift : 2;
                    bool ctrl  : 2;
                    bool alt   : 2;
                    bool cmd   : 2;
                };
            };

            bool numlck;
            bool capslck;
            bool scrllck;
        };

        struct ActionState {
            ui32         time;
            KeyModifiers modifiers;
        };

        struct ButtonState {
            bool        is_pressed;
            ui32        press_count;
            ActionState last_press, last_release;
        };
    }  // namespace ui
}  // namespace hemlock
namespace hui = hemlock::ui;

#endif  // __hemlock_ui_input_state_hpp

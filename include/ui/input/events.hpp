#ifndef __hemlock_ui_input_state_window_state_hpp
#define __hemlock_ui_input_state_window_state_hpp

#include "ui/input/keys.hpp"
#include "ui/input/state.hpp"

namespace hemlock {
    namespace ui {
        struct InputEvent {
            CommonMouseState mouse;
            KeyModifiers     modifiers;
        };

        struct MouseEvent : InputEvent {
            MouseCoords coords;
        };
        struct MouseButtonEvent : MouseEvent {
            hui::MouseButton name;
            ui8 button_id, presses;
        };
        struct MouseWheelScrollEvent : MouseEvent {
            i32 dx, dy;
        };
        struct MouseMoveEvent : MouseEvent {
            i32 dx, dy;
        };

        struct KeyboardButtonEvent : InputEvent {
            PhysicalKey physical_key;
            VirtualKey  virtual_key;
            ButtonState state;
            ui8         presses;
        };

        struct TextInputEvent : InputEvent {
            union {
                ui8    text[32];
                ui16  wtext[16];
                ui32 wwtext[8];
            };
        };
        struct TextEditingEvent : TextInputEvent {
            ui32 start;
            ui32 length;
        };
                
        struct TextDropEvent {
            std::string text;
        };
        struct FileDropEvent {
            std::string filename;
        };

        struct WindowEvent {
            ui32 window_id;
        };

        struct WindowResizeEvent : WindowEvent {
            i32 width, height;
        };

        struct MouseEvents {
            Event<MouseMoveEvent>        move;
            Event<MouseButtonEvent>      button_down;
            Event<MouseButtonEvent>      button_up;
            Event<MouseWheelScrollEvent> wheel_scroll;
        };

        struct KeyboardEvents {
            Event<KeyboardButtonEvent> button_down;
            Event<KeyboardButtonEvent> button_up;
        };

        struct WindowEvents {
            Event<WindowEvent>       mouse_enter;
            Event<WindowEvent>       mouse_exit;
            Event<WindowEvent>       focus_gained;
            Event<WindowEvent>       focus_lost;
            Event<WindowEvent>       close;
            Event<WindowResizeEvent> resize;
        };

        struct TextEvents {
            Event<TextInputEvent>   input;
            Event<TextEditingEvent> editing;
        };

        struct DropEvents {
            Event<>              begin;
            Event<>              complete;
            Event<FileDropEvent> file;
            Event<TextDropEvent> text;
        };
    }
}
namespace hui = hemlock::ui;

#endif // __hemlock_ui_input_state_window_state_hpp

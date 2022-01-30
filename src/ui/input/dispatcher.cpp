#include "stdafx.h"

#include "app/app.h"
#include "ui/input/manager.h"

#include "ui/input/dispatcher.h"

static hui::MouseButton convert_mouse_button(ui8 sdl_button) {
    switch (sdl_button) {
        case SDL_BUTTON_LEFT:
            return hui::MouseButton::LEFT;
            break;
        case SDL_BUTTON_RIGHT:
            return hui::MouseButton::RIGHT;
            break;
        case SDL_BUTTON_MIDDLE:
            return hui::MouseButton::MIDDLE;
            break;
        default:
            return hui::MouseButton::UNKNOWN;
    }
}

hui::InputDispatcher* hui::InputDispatcher::m_instance = nullptr;

void hui::InputDispatcher::init(happ::IApp* app, hui::InputManager* manager) {
    if (m_initialised) return;
    m_initialised = true;

    m_app     = app;
    m_manager = manager;

    SDL_SetEventFilter(hui::InputDispatcher::handle_event, (void*)this);
    SDL_EventState(SDL_DROPFILE,     SDL_ENABLE);
    SDL_EventState(SDL_DROPBEGIN,    SDL_ENABLE);
    SDL_EventState(SDL_DROPCOMPLETE, SDL_ENABLE);
    SDL_EventState(SDL_DROPTEXT,     SDL_ENABLE);
}

void hui::InputDispatcher::dispose() {
    if (!m_initialised) return;
    m_initialised = false;

    SDL_SetEventFilter(nullptr, nullptr);

    m_app     = nullptr;
    m_manager = nullptr;
}

void hui::InputDispatcher::set_text_mode(bool on) {
    if (on) {
        SDL_StartTextInput();
    } else {
        SDL_StopTextInput();
    }
}

// Return 1 for unknown input, 0 for handled input.
i32 hui::InputDispatcher::handle_event(void* data, SDL_Event* event) {
    InputDispatcher* dispatcher = static_cast<InputDispatcher*>(data);
    switch (event->type) {
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            MouseButtonEvent mbe;
            mbe.coords    = { event->button.x, event->button.y };
            mbe.name      = convert_mouse_button(event->button.button);
            mbe.button_id = event->button.button - 1;
            mbe.presses   = event->button.clicks;
            mbe.mouse     = dispatcher->m_manager->common_mouse_state();
            mbe.modifiers = dispatcher->m_manager->key_modifier_state();
            if (event->button.type == SDL_MOUSEBUTTONDOWN) {
                dispatcher->on_mouse.button_down(mbe);
                dispatcher->m_manager->press(mbe.button_id);
            } else {
                dispatcher->on_mouse.button_up(mbe);
                dispatcher->m_manager->release(mbe.button_id);
            }
            break;
        case SDL_MOUSEMOTION:
            MouseMoveEvent mme;
            mme.coords    = { event->motion.x, event->motion.y };
            mme.dx        = event->motion.xrel;
            mme.dy        = event->motion.yrel;
            mme.mouse     = dispatcher->m_manager->common_mouse_state();
            mme.modifiers = dispatcher->m_manager->key_modifier_state();
            dispatcher->on_mouse.move(mme);
            dispatcher->m_manager->set_mouse_coords(mme.coords);
            break;
        case SDL_MOUSEWHEEL:
            MouseWheelScrollEvent mwse;
            mwse.coords    = dispatcher->m_manager->mouse_coords();
            mwse.dx        = event->wheel.x;
            mwse.dy        = event->wheel.y;
            mwse.mouse     = dispatcher->m_manager->common_mouse_state();
            mwse.modifiers = dispatcher->m_manager->key_modifier_state();
            if (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
                mwse.dx *= -1;
                mwse.dy *= -1;
            }
            dispatcher->on_mouse.wheel_scroll(mwse);
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            KeyboardButtonEvent kbe;
            kbe.physical_key = (PhysicalKey)event->key.keysym.scancode;
            kbe.virtual_key  = (VirtualKey)event->key.keysym.sym;
            kbe.presses      = event->key.repeat;
            kbe.mouse        = dispatcher->m_manager->common_mouse_state();
            kbe.modifiers    = dispatcher->m_manager->key_modifier_state();
            if (event->key.type == SDL_KEYDOWN) {
                dispatcher->m_manager->press(kbe.physical_key);
                kbe.state = dispatcher->m_manager->keyboard_key_state(kbe.physical_key);
                dispatcher->on_keyboard.button_down(kbe);
            } else {
                dispatcher->m_manager->release(kbe.physical_key);
                kbe.state = dispatcher->m_manager->keyboard_key_state(kbe.physical_key);
                dispatcher->on_keyboard.button_up(kbe);
            }
            break;
        case SDL_QUIT:
            dispatcher->m_app->set_should_quit(true);

            dispatcher->on_quit();

            if (dispatcher->m_app->should_quit()) {
                SDL_Quit();
                exit(0);
            }
            break;
        case SDL_WINDOWEVENT:
            switch (event->window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    dispatcher->on_window.close({event->window.windowID});
                    break;
                case SDL_WINDOWEVENT_ENTER:
                    dispatcher->on_window.mouse_enter({event->window.windowID});
                    dispatcher->m_manager->set_mouse_focus(event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    dispatcher->on_window.mouse_exit({event->window.windowID});
                    dispatcher->m_manager->set_mouse_focus(-1);
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    dispatcher->on_window.focus_gained({event->window.windowID});
                    dispatcher->m_manager->set_keyboard_focus(event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    dispatcher->on_window.focus_lost({event->window.windowID});
                    dispatcher->m_manager->set_keyboard_focus(-1);
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    WindowResizeEvent mre;
                    mre.window_id = event->window.windowID;
                    mre.width     = event->window.data1;
                    mre.height    = event->window.data2;
                    dispatcher->on_window.resize(mre);
                    break;
                default:
                    return 1;
            }
            break;
        case SDL_TEXTINPUT:
            TextInputEvent tie;
            std::memcpy(tie.text, event->text.text, 32);
            tie.mouse     = dispatcher->m_manager->common_mouse_state();
            tie.modifiers = dispatcher->m_manager->key_modifier_state();
            dispatcher->on_text.input(tie);
            break;
        case SDL_TEXTEDITING:
            TextEditingEvent tee;
            std::memcpy(tee.text, event->edit.text, 32);
            tee.start     = event->edit.start;
            tee.length    = event->edit.length;
            tee.mouse     = dispatcher->m_manager->common_mouse_state();
            tee.modifiers = dispatcher->m_manager->key_modifier_state();
            dispatcher->on_text.editing(tee);
            break;
        case SDL_DROPBEGIN:
            dispatcher->on_drop.begin();
            break;
        case SDL_DROPCOMPLETE:
            dispatcher->on_drop.complete();
            break;
        case SDL_DROPFILE:
            {
                FileDropEvent fde;
                fde.filename = event->drop.file;
                dispatcher->on_drop.file(fde);
                SDL_free(event->drop.file);
            }
            break;
        case SDL_DROPTEXT:
            {
                TextDropEvent tde;
                tde.text = event->drop.file;
                dispatcher->on_drop.text(tde);
                SDL_free(event->drop.file);
            }
            break;
        default:
            return 1;
    }
    return 0;
}

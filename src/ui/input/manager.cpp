#include "stdafx.h"

#include "ui/input/manager.h"

void hui::InputManager::init() {
    m_mouse_focus    = -1;
    m_keyboard_focus = -1;

    m_mouse_coords = {};
    std::fill_n(&m_mouse_button_state[0], 255, hui::ButtonState{});

    m_key_modifier_state = {};
}

void hui::InputManager::dispose() {
    m_mouse_focus    = -1;
    m_keyboard_focus = -1;

    m_mouse_coords = {};
    std::fill_n(&m_mouse_button_state[0], 255, hui::ButtonState{});

    m_key_modifier_state = {};

    KeyboardKeyStates().swap(m_keyboard_key_state);
}

hui::CommonMouseState hui::InputManager::common_mouse_state() {
    CommonMouseState cms;

    cms.left  = m_mouse_button_state[static_cast<ui8>(MouseButton::LEFT)].is_pressed;
    cms.right = m_mouse_button_state[static_cast<ui8>(MouseButton::RIGHT)].is_pressed;
    cms.middle
        = m_mouse_button_state[static_cast<ui8>(MouseButton::MIDDLE)].is_pressed;

    return cms;
}

hui::ButtonState hui::InputManager::keyboard_key_state(PhysicalKey key) {
    auto it = m_keyboard_key_state.find(key);
    if (it == m_keyboard_key_state.end()) return {};
    return (*it).second;
}

bool hui::InputManager::is_pressed(PhysicalKey key) {
    auto it = m_keyboard_key_state.find(key);
    if (it == m_keyboard_key_state.end()) return false;
    return (*it).second.is_pressed;
}

void hui::InputManager::press(ui8 button) {
    m_mouse_button_state[button].is_pressed           = true;
    m_mouse_button_state[button].press_count          += 1;
    m_mouse_button_state[button].last_press.time      = SDL_GetTicks();
    m_mouse_button_state[button].last_press.modifiers = m_key_modifier_state;
}

void hui::InputManager::release(ui8 button) {
    m_mouse_button_state[button].is_pressed             = false;
    m_mouse_button_state[button].last_release.time      = SDL_GetTicks();
    m_mouse_button_state[button].last_release.modifiers = m_key_modifier_state;
}

void hui::InputManager::press(PhysicalKey key) {
    switch (key) {
        case PhysicalKey::H_LSHIFT:
            m_key_modifier_state.lshift = true;
            break;
        case PhysicalKey::H_RSHIFT:
            m_key_modifier_state.rshift = true;
            break;
        case PhysicalKey::H_LCTRL:
            m_key_modifier_state.lctrl = true;
            break;
        case PhysicalKey::H_RCTRL:
            m_key_modifier_state.rctrl = true;
            break;
        case PhysicalKey::H_LALT:
            m_key_modifier_state.lalt = true;
            break;
        case PhysicalKey::H_RALT:
            m_key_modifier_state.ralt = true;
            break;
        case PhysicalKey::H_LCMD:
            m_key_modifier_state.lcmd = true;
            break;
        case PhysicalKey::H_RCMD:
            m_key_modifier_state.rcmd = true;
            break;
        case PhysicalKey::H_NUMLOCKCLEAR:
            m_key_modifier_state.numlck = !m_key_modifier_state.numlck;
            break;
        case PhysicalKey::H_CAPSLOCK:
            m_key_modifier_state.capslck = !m_key_modifier_state.capslck;
            break;
        case PhysicalKey::H_SCROLLLOCK:
            m_key_modifier_state.scrllck = !m_key_modifier_state.scrllck;
            break;
        default:
            break;
    }

    auto it = m_keyboard_key_state.find(key);
    if (it == m_keyboard_key_state.end()) {
        ButtonState key_state;
        key_state.is_pressed  = true;
        key_state.press_count = 1;
        key_state.last_press
            = { .time = SDL_GetTicks(), .modifiers = m_key_modifier_state };
        key_state.last_release    = { .time = 0, .modifiers = {} };
        m_keyboard_key_state[key] = key_state;
    } else {
        ButtonState& key_state         = (*it).second;
        key_state.is_pressed           = true;
        key_state.press_count          += 1;
        key_state.last_press.time      = SDL_GetTicks();
        key_state.last_press.modifiers = m_key_modifier_state;
    }
}

void hui::InputManager::release(PhysicalKey key) {
    switch (key) {
        case PhysicalKey::H_LSHIFT:
            m_key_modifier_state.lshift = false;
            break;
        case PhysicalKey::H_RSHIFT:
            m_key_modifier_state.rshift = false;
            break;
        case PhysicalKey::H_LCTRL:
            m_key_modifier_state.lctrl = false;
            break;
        case PhysicalKey::H_RCTRL:
            m_key_modifier_state.rctrl = false;
            break;
        case PhysicalKey::H_LALT:
            m_key_modifier_state.lalt = false;
            break;
        case PhysicalKey::H_RALT:
            m_key_modifier_state.ralt = false;
            break;
        case PhysicalKey::H_LCMD:
            m_key_modifier_state.lcmd = false;
            break;
        case PhysicalKey::H_RCMD:
            m_key_modifier_state.rcmd = false;
            break;
        default:
            break;
    }

    auto it = m_keyboard_key_state.find(key);
    if (it == m_keyboard_key_state.end()) {
        ButtonState key_state;
        key_state.is_pressed  = false;
        key_state.press_count = 0;
        key_state.last_press  = { .time = 0, .modifiers = {} };
        key_state.last_release
            = { .time = SDL_GetTicks(), .modifiers = m_key_modifier_state };
        m_keyboard_key_state[key] = key_state;
    } else {
        ButtonState& key_state           = (*it).second;
        key_state.is_pressed             = false;
        key_state.last_release.time      = SDL_GetTicks();
        key_state.last_release.modifiers = m_key_modifier_state;
    }
}

#include "stdafx.h"

#include "graphics/window_manager.h"

hg::WindowError hg::WindowManager::init(hemlock::app::IApp* app) {
    if (m_main_window != nullptr) return WindowError::NONE;

    m_app = app;

    auto [ window, err ] = add_window();
    if (err != WindowError::NONE) return err;

    m_main_window = window;
    m_windows.insert({m_main_window->window_id(), m_main_window});

    return WindowError::NONE;
}

void hg::WindowManager::dispose() {
    for (auto& window : m_windows) {
        window.second->dispose();
        delete window.second;
    }
    Windows().swap(m_windows);
    m_main_window = nullptr;
}

bool hg::WindowManager::set_main_window(Window* window) {
    auto it = std::find_if(m_windows.begin(), m_windows.end(), [window](const auto& rhs) {
        return rhs.second == window;
    });

    if (it == m_windows.end()) return false;

    m_main_window = window;

    return true;
}

bool hg::WindowManager::set_main_window(ui32 window_id) {
    auto it =  m_windows.find(window_id);

    if (it == m_windows.end()) return false;

    m_main_window = (*it).second;

    return true;
}

std::pair<hg::Window*, hg::WindowError>
hg::WindowManager::add_window(WindowSettings settings /*= {}*/) {
    hg::Window* new_window = new hg::Window();
    WindowError err = new_window->init(settings);

    m_windows.insert({new_window->window_id(), new_window});

    return { new_window, err };
}

bool hg::WindowManager::add_window(CALLEE_DELETE Window* window) {
    return m_windows.try_emplace(window->window_id(), window).second;
}

bool hg::WindowManager::dispose_window(Window* window) {
    ui64 erased = std::erase_if(m_windows, [window](const auto& rhs) {
        return rhs.second == window;
    });

    if (erased == 0) return false;

    window->dispose();
    delete window;

    return true;
}

void hg::WindowManager::sync_windows() {
    for (auto& window : m_windows) window.second->sync();
}
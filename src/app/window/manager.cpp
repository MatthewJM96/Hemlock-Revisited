#include "stdafx.h"

#include "app/app.h"
#include "app/window/manager.h"

happ::WindowManager::WindowManager() :
    WindowManagerBase(),
    handle_window_close([&](Sender, hui::WindowEvent event) {
        dispose_window(event.window_id);
    }),
    m_quit_on_main_window_close(true) { /* Empty. */
}

happ::WindowError happ::WindowManager::init(hemlock::app::AppBase* app) {
    if (m_main_window != nullptr) return WindowError::NONE;

    m_app = app;

    hui::InputDispatcher::instance()->on_window.close += &handle_window_close;

    auto [window, err] = add_window();
    if (err != WindowError::NONE) return err;

    m_main_window = window;
    m_windows.insert({ m_main_window->window_id(), m_main_window });

    return WindowError::NONE;
}

void happ::WindowManager::dispose() {
    for (auto& window : m_windows) {
        window.second->dispose();
        delete window.second;
    }
    Windows().swap(m_windows);
    m_main_window = nullptr;
    m_app         = nullptr;
}

bool happ::WindowManager::set_main_window(Window* window) {
    auto it
        = std::find_if(m_windows.begin(), m_windows.end(), [window](const auto& rhs) {
              return rhs.second == window;
          });

    if (it == m_windows.end()) return false;

    m_main_window = window;

    return true;
}

bool happ::WindowManager::set_main_window(ui32 window_id) {
    auto it = m_windows.find(window_id);

    if (it == m_windows.end()) return false;

    m_main_window = (*it).second;

    return true;
}

std::pair<happ::Window*, happ::WindowError>
happ::WindowManager::add_window(WindowSettings settings /*= {}*/) {
    happ::Window* new_window = new happ::Window();
    WindowError   err        = new_window->init(settings);

    m_windows.insert({ new_window->window_id(), new_window });

    return { new_window, err };
}

bool happ::WindowManager::add_window(CALLEE_DELETE Window* window) {
    return m_windows.try_emplace(window->window_id(), window).second;
}

bool happ::WindowManager::dispose_window(Window* window) {
    ui64 erased = std::erase_if(m_windows, [window](const auto& rhs) {
        return rhs.second == window;
    });

    if (erased == 0) return false;

    if ((m_main_window == window && m_quit_on_main_window_close)
        || m_windows.size() <= 0)
    {
        m_app->set_should_quit();
    }

    window->dispose();
    delete window;

    return true;
}

bool happ::WindowManager::dispose_window(ui32 window_id) {
    auto it = m_windows.find(window_id);

    if (it == m_windows.end()) return false;

    Window* window = (*it).second;

    m_windows.erase(it);

    if ((m_main_window == window && m_quit_on_main_window_close)
        || m_windows.size() <= 0)
    {
        m_app->set_should_quit();
    }

    window->dispose();
    delete window;

    return true;
}

void happ::WindowManager::sync_windows() {
    for (auto& window : m_windows) window.second->sync();
}

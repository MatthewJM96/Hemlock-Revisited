#include "stdafx.h"

#include "app/screen_base.h"
#include "app/window/window_base.h"

#include "app/process/process_base.h"

happ::ProcessBase::ProcessBase()  :
    m_initialised(false),
    m_should_end_process(false),
    m_current_screen(nullptr),
    m_window(nullptr)
{ /* Empty */ }

void happ::ProcessBase::init() {
    if (m_initialised) return;
    m_initialised = true;

    m_should_end_process = false;

    prepare_window();

    prepare_screens();
}

void happ::ProcessBase::dispose() {
    if (!m_initialised) return;
    m_initialised = false;

    m_current_screen = nullptr;
    dispose_screens();

    m_window->dispose();
    delete m_window;
    m_window = nullptr;
}

bool happ::ProcessBase::add_screen(Screen&& screen) {
    auto [_, added] = m_screens.insert(std::forward<Screen>(screen));
    if (!added) return false;

    screen.second->init(screen.first, this);

    return true;
}

bool happ::ProcessBase::go_to_screen(const std::string& name, FrameTime time) {
    auto it = m_screens.find(name);
    if (it == m_screens.end()) return false;

    if (m_current_screen != nullptr) {
        m_current_screen->set_next_screen((*it).second);
        ((*it).second)->set_prev_screen(m_current_screen);

        m_current_screen->end(time);
    }

    auto tmp = m_current_screen;
    m_current_screen = (*it).second;

    m_current_screen->start(time);

    on_screen_change({ tmp, m_current_screen });

    return true;
}

void happ::ProcessBase::set_should_end_process(bool should_end_process /*= true*/) {
    m_should_end_process = should_end_process;
    if (should_end_process) on_process_end();
}

bool happ::ProcessBase::handle_screen_requests() {
    if (m_current_screen == nullptr) {
        set_should_end_process();
        return false;
    }

    switch (m_current_screen->state()) {
        case ScreenState::CHANGE_NEXT:
            m_current_screen->end(m_timer->frame_times().back());

            goto_next_screen();

            if (m_current_screen != nullptr) {
                m_current_screen->start(m_timer->frame_times().back());
            } else {
                set_should_end_process();
                return false;
            }
            return true;

        case ScreenState::CHANGE_PREV:
            m_current_screen->end(m_timer->frame_times().back());

            goto_prev_screen();

            if (m_current_screen != nullptr) {
                m_current_screen->start(m_timer->frame_times().back());
            } else {
                set_should_end_process();
                return false;
            }
            return true;

        case ScreenState::QUIT_APPLICATION:
            set_should_end_process();
            return false;

        case ScreenState::NONE:
            // Screen is doing nothing, but we shouldn't quit either.
            return false;

        case ScreenState::RUNNING:
            // Screen is running as usual.
            return true;

        default:
            end_process();
    }
    // We should never get here...
    return false;
}

void happ::ProcessBase::goto_next_screen() {
    if (m_current_screen == nullptr) return;

    ScreenBase* next_screen = m_current_screen->next_screen();

    ScreenBase* tmp = m_current_screen;

    if (next_screen == nullptr) {
        m_current_screen = nullptr;
    } else {
        auto it = m_screens.find(next_screen->name());

        if (it == m_screens.end()) {
            m_current_screen->set_next_screen(nullptr);
            m_current_screen = nullptr; // TODO(Matthew): Instead of nullptr, maybe a default object that at least linked back to previous screen?
        } else {
            m_current_screen = (*it).second;
            m_current_screen->set_prev_screen(tmp);
        }
    }

    on_screen_change({ tmp, m_current_screen });
}

void happ::ProcessBase::goto_prev_screen() {
    if (m_current_screen == nullptr) return;

    ScreenBase* prev_screen = m_current_screen->prev_screen();

    ScreenBase* tmp = m_current_screen;

    if (prev_screen == nullptr) {
        m_current_screen = nullptr;
    } else {
        auto it = m_screens.find(prev_screen->name());

        if (it == m_screens.end()) {
            m_current_screen->set_prev_screen(nullptr);
            m_current_screen = nullptr; // TODO(Matthew): Instead of nullptr, maybe a default object that at least linked back to previous screen?
        } else {
            m_current_screen = (*it).second;
            m_current_screen->set_next_screen(tmp);
        }
    }

    on_screen_change({ tmp, m_current_screen });
}

void happ::ProcessBase::dispose_screens() {
    for (auto& screen : m_screens) {
        screen.second->dispose();
        delete screen.second;
    }
    Screens().swap(m_screens);
}

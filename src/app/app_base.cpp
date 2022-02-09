#include "stdafx.h"

#include "timing.h"
#include "app/screen.h"
#include "app/window/manager.h"
#include "ui/input/dispatcher.h"
#include "ui/input/manager.h"

#include "app/app_base.h"

happ::AppBase::AppBase()  :
    handle_external_quit([&](Sender) {
        set_should_quit();
    }),
    m_current_screen(nullptr),
    m_window_manager(nullptr)
{ /* Empty */ }

void happ::AppBase::set_should_quit(bool should_quit /*= true*/) {
    m_should_quit = should_quit;
    if (should_quit) on_quit();
}

bool happ::AppBase::change_screen(std::string name) {
    auto it = m_screens.find(name);
    if (it == m_screens.end()) return false;

    if (m_current_screen != nullptr) {
        m_current_screen->set_next_screen((*it).second);
        ((*it).second)->set_prev_screen(m_current_screen);
    }

    auto tmp = m_current_screen;
    m_current_screen = (*it).second;

    on_screen_change({ tmp, m_current_screen });

    return true;
}

void happ::AppBase::quit() {
    m_current_screen->end(m_previous_times);

    dispose();

    SDL_Quit();
    exit(0);
}

bool happ::AppBase::add_screen(Screen screen) {
    auto [_, added] = m_screens.insert(screen);
    if (!added) return false;

    screen.second->init(screen.first, this);

    return true;
}

bool happ::AppBase::handle_screen_requests() {
    if (m_current_screen == nullptr) {
        set_should_quit();
        return false;
    }

    switch (m_current_screen->state()) {
        case ScreenState::CHANGE_NEXT:
            m_current_screen->end(m_current_times);

            goto_next_screen();

            if (m_current_screen != nullptr) {
                m_current_screen->start(m_current_times);
            } else {
                set_should_quit();
                return false;
            }
            return true;

        case ScreenState::CHANGE_PREV:
            m_current_screen->end(m_current_times);

            goto_prev_screen();

            if (m_current_screen != nullptr) {
                m_current_screen->start(m_current_times);
            } else {
                set_should_quit();
                return false;
            }
            return true;

        case ScreenState::QUIT_APPLICATION:
            set_should_quit();
            return false;

        case ScreenState::NONE:
            // Screen is doing nothing, but we shouldn't quit either.
            return false;

        case ScreenState::RUNNING:
            // Screen is running as usual.
            return true;

        default:
            quit();
    }
    // We should never get here...
    return false;
}

void happ::AppBase::goto_next_screen() {
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

void happ::AppBase::goto_prev_screen() {
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

void happ::AppBase::dispose_screens() {
    for (auto& screen : m_screens) {
        screen.second->dispose();
    }
    ScreenList().swap(m_screens);
}

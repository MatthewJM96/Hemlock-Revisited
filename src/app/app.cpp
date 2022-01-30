#include "stdafx.h"

#include "timing.h"
#include "app/screen.h"
#include "graphics/window_manager.h"

#include "app/app.h"

bool happ::IApp::change_screen(std::string name) {
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

void happ::IApp::quit() {
    m_current_screen->end(m_previous_times);
    dispose();
}

bool happ::IApp::add_screen(Screen screen) {
    auto [_, added] = m_screens.insert(screen);
    if (!added) return false;

    screen.second->init(screen.first);

    return true;
}

bool happ::IApp::handle_requests() {
    switch (m_current_screen->state()) {
        case ScreenState::CHANGE_NEXT:
            m_current_screen->end(m_current_times);

            goto_next_screen();

            if (m_current_screen != nullptr) {
                m_current_screen->start(m_current_times);
            }
            break;

        case ScreenState::CHANGE_PREV:
            m_current_screen->end(m_current_times);

            goto_prev_screen();

            if (m_current_screen != nullptr) {
                m_current_screen->start(m_current_times);
            }
            break;

        case ScreenState::EXIT_APPLICATION:
            m_should_quit = true;
            break;

        default:
            return false;
    }
    return true;
}

void happ::IApp::goto_next_screen() {
    if (m_current_screen == nullptr) return;

    IScreen* next_screen = m_current_screen->next_screen();

    IScreen* tmp = m_current_screen;

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

void happ::IApp::goto_prev_screen() {
    if (m_current_screen == nullptr) return;

    IScreen* prev_screen = m_current_screen->prev_screen();

    IScreen* tmp = m_current_screen;

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

void happ::IApp::dispose_screens() {
    for (auto& screen : m_screens) {
        screen.second->dispose();
    }
    ScreenList().swap(m_screens);
}

void happ::BasicApp::init() {
    if (m_initialised) return;
    m_initialised = true;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    m_window_manager = new hg::WindowManager();
    if (m_window_manager->init() != hg::WindowError::NONE) {
        puts("Window could not be initialised...\n");
        exit(1);
    }

    m_input_manager = new hui::InputManager();
    hui::InputDispatcher::instance()->init(m_window, m_input_manager);

    #ifdef HEMLOCK_USE_DEVIL
    ilutRenderer(ILUT_OPENGL);
    ilutEnable(ILUT_OPENGL_CONV); // TODO(Matthew): Make this optional, some projects may consider on-board texture conversions fine.
    #endif

    m_current_times  = {};
    m_previous_times = {};

    m_fps_limiter = new FpsLimiter();
    m_fps_limiter->init(60.0); // TODO(Matthew): Get max FPS from user preferences.

    prepare_screens();
}

void happ::BasicApp::dispose() {
    if (!m_initialised) return;
    m_initialised = false;

    dispose_screens();
    m_current_screen = nullptr;

    m_window_manager->dispose();
    delete m_window_manager;
    m_window_manager = nullptr;

    m_input_manager->dispose();
    delete m_input_manager;
    m_input_manager = nullptr;
}

void happ::BasicApp::run() {
    init();

    while (!m_should_quit) {
        m_fps_limiter->begin();

        calculate_times();

        SDL_PumpEvents();

        handle_requests();

        m_current_screen->update(m_current_times);

        if (!handle_requests()) {
            m_current_screen->draw(m_current_times);
        }

        m_window_manager->sync_windows();
        
#if defined(OUTPUT_FPS)
        static i32 i = 0;
        if (i++ % 20 == 0) {
            std::cout << "FPS: " << m_fps_limiter->getFPS() << std::endl;
        }
#endif

        m_fps_limiter->end();
    }
}

void happ::BasicApp::calculate_times() {
    m_previous_times = m_current_times;

    m_current_times.frame  = m_fps_limiter->frame_time();
    m_current_times.total += m_current_times.frame;
}

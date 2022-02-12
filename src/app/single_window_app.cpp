#include "stdafx.h"

#include "timing.h"
#include "app/screen_base.h"
#include "app/window/window.h"
#include "ui/input/dispatcher.h"
#include "ui/input/manager.h"

#include "app/single_window_app.h"

void happ::SingleWindowApp::init() {
    m_input_manager = new hui::InputManager();
    hui::InputDispatcher::instance()->init(m_input_manager);
    hui::InputDispatcher::instance()->on_quit += &handle_external_quit;

    m_fps_limiter = new FpsLimiter();
    m_fps_limiter->init(60.0); // TODO(Matthew): Get max FPS from user preferences.

#ifdef HEMLOCK_USING_DEVIL
    ilutRenderer(ILUT_OPENGL);
    ilutEnable(ILUT_OPENGL_CONV); // TODO(Matthew): Make this optional, some projects may consider on-board texture conversions fine.
#endif

    ProcessBase::init();
}

void happ::SingleWindowApp::dispose() {
    m_input_manager->dispose();
    delete m_input_manager;
    m_input_manager = nullptr;

    ProcessBase::dispose();

#ifdef HEMLOCK_USING_SDL_TTF
    TTF_Quit();
#endif // HEMLOCK_USING_SDL_TTF
}

void happ::SingleWindowApp::run() {
    init();

    while (!m_should_quit) {
        m_fps_limiter->begin();

        calculate_times();

        SDL_PumpEvents();

        // Update screen if it is in a state ready to act.
        if (handle_screen_requests()) {
            m_current_screen->update(m_current_times);

            // Draw screen if it is still in a state ready to
            // act after the update phase.
            if (handle_screen_requests()) {
                m_current_screen->draw(m_current_times);
            }
        }

        m_window->sync();
        
#if defined(OUTPUT_FPS)
        static i32 i = 0;
        if (i++ % 20 == 0) {
            std::cout << "FPS: " << m_fps_limiter->getFPS() << std::endl;
        }
#endif

        m_fps_limiter->end();
    }
}

void happ::SingleWindowApp::prepare_window() {
#ifdef HEMLOCK_USING_SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif // HEMLOCK_USING_SDL

#ifdef HEMLOCK_USING_SDL_TTF
    if (TTF_Init() < 0) {
        debug_printf("TTF font library could not be initialised...\n");
        exit(1);
    }
#endif // HEMLOCK_USING_SDL_TTF

    m_window = new happ::Window();
    if (m_window->init() != happ::WindowError::NONE) {
        debug_printf("Window could not be initialised...\n");
        exit(2);
    }

#ifdef HEMLOCK_USING_OPENGL
    debug_printf("*** OpenGL Version:  %s ***\n", glGetString(GL_VERSION));
    debug_printf("*** OpenGL Renderer: %s ***\n", glGetString(GL_RENDERER));

    GLint nr_attributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nr_attributes);
    debug_printf("Maximum # of vertex attributes supported: %d.\n", nr_attributes);
#endif // HEMLOCK_USING_OPENGL
}

void happ::SingleWindowApp::calculate_times() {
    m_previous_times = m_current_times;

    m_current_times.frame  = m_fps_limiter->frame_time();
    m_current_times.total += m_current_times.frame;
}

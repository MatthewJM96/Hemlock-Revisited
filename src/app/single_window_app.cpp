#include "stdafx.h"

#include "timing.h"
#include "app/screen.h"
#include "app/window/window_base.h"
#include "ui/input/dispatcher.h"
#include "ui/input/manager.h"

#include "app/single_window_app.h"

void happ::SingleWindowApp::init() {
    if (m_initialised) return;
    m_initialised = true;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (TTF_Init() < 0) {
        debug_printf("TTF font library could not be initialised...\n");
        exit(1);
    }

    debug_printf("*** OpenGL Version:  %s ***\n", glGetString(GL_VERSION));
    debug_printf("*** OpenGL Renderer: %s ***\n", glGetString(GL_RENDERER));

    GLint nr_attributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nr_attributes);
    debug_printf("Maximum # of vertex attributes supported: %d.\n", nr_attributes);

    m_window_manager = new happ::WindowManager();
    if (m_window_manager->init(this) != happ::WindowError::NONE) {
        debug_printf("Window could not be initialised...\n");
        exit(2);
    }

    m_input_manager = new hui::InputManager();
    hui::InputDispatcher::instance()->init(m_input_manager);
    hui::InputDispatcher::instance()->on_quit += &handle_external_quit;

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

void happ::SingleWindowApp::dispose() {
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

    TTF_Quit();
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

void happ::SingleWindowApp::calculate_times() {
    m_previous_times = m_current_times;

    m_current_times.frame  = m_fps_limiter->frame_time();
    m_current_times.total += m_current_times.frame;
}

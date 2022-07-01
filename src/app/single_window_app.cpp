#include "stdafx.h"

#include "app/screen_base.h"
#include "app/window/window.h"
#include "ui/input/dispatcher.h"
#include "ui/input/manager.h"

#include "app/single_window_app.h"

void happ::SingleWindowApp::init() {
    init(new FrameTimer());
}

void happ::SingleWindowApp::init(FrameTimer* timer) {
    m_timer = timer;

    hui::InputDispatcher::instance()->init(&m_input_manager);
    hui::InputDispatcher::instance()->on_quit += &handle_external_quit;

#if defined(HEMLOCK_USING_DEVIL)
    ilutRenderer(ILUT_OPENGL);
    ilutEnable(ILUT_OPENGL_CONV); // TODO(Matthew): Make this optional, some projects may consider on-board texture conversions fine.
#endif

    ProcessBase::init();
}

void happ::SingleWindowApp::init(f32 target_fps, size_t tracked_frames_count /*= 5*/) {
    init(new FrameLimiter(tracked_frames_count, target_fps));
}

void happ::SingleWindowApp::dispose() {
    m_input_manager.dispose();

    delete m_timer;
    m_timer = nullptr;

    ProcessBase::dispose();

#if defined(HEMLOCK_USING_SDL_TTF)
    TTF_Quit();
#endif // defined(HEMLOCK_USING_SDL_TTF)
}

void happ::SingleWindowApp::run() {
    init();

    m_timer->start();

    while (!m_should_quit) {
        SDL_PumpEvents();

        // Update screen if it is in a state ready to act.
        if (handle_screen_requests()) {
            m_current_screen->update(m_timer->frame_times().back());

            // Draw screen if it is still in a state ready to
            // act after the update phase.
            if (handle_screen_requests()) {
                m_current_screen->draw(m_timer->frame_times().back());
            }
        }

        m_window->sync();
        
#if defined(OUTPUT_FPS)
        static i32 i = 0;
        if (i++ % 20 == 0) {
            printf("FPS: %f\n", m_timer->fps());
        }
#endif

        m_timer->frame_end();
    }

    // TODO(Matthew): need to make sure thread pools are properly cleaned up.
    // dispose();
}

void happ::SingleWindowApp::prepare_window() {
#if defined(HEMLOCK_USING_SDL)
    if (
        SDL_Init(
            SDL_INIT_TIMER
          | SDL_INIT_VIDEO
          | SDL_INIT_GAMECONTROLLER
        ) < 0
    ) {
        debug_printf("Couldn't initialise SDL.\n");
        debug_printf("%s\n", SDL_GetError());
        exit(1);
    }
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif // defined(HEMLOCK_USING_SDL)

#if defined(HEMLOCK_USING_SDL_TTF)
    if (TTF_Init() < 0) {
        debug_printf("TTF font library could not be initialised...\n");
        exit(2);
    }
#endif // defined(HEMLOCK_USING_SDL_TTF)

    m_window = new happ::Window();
    if (m_window->init() != happ::WindowError::NONE) {
        debug_printf("Window could not be initialised...\n");
        exit(3);
    }

#if defined(HEMLOCK_USING_OPENGL)
    debug_printf("*** OpenGL Version:  %s ***\n", glGetString(GL_VERSION));
    debug_printf("*** OpenGL Renderer: %s ***\n", glGetString(GL_RENDERER));

    GLint nr_attributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nr_attributes);
    debug_printf("Maximum # of vertex attributes supported: %d.\n", nr_attributes);
#endif // defined(HEMLOCK_USING_OPENGL)
}

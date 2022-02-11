#include "stdafx.h"

#include "ui/input/dispatcher.h"

#include "app/window/window.h"

happ::Window::Window() :
    WindowBase(),
    m_window(nullptr),
    m_context(nullptr)
{ /* Empty. */ }

happ::WindowError happ::Window::init(WindowSettings settings /*= {}*/) {
    if (m_initialised) return WindowError::NONE;
    m_initialised = true;

    m_settings = settings;

    determine_modes();

#ifdef HEMLOCK_USING_SDL
#ifdef HEMLOCK_USING_OPENGL
    ui32 flags = SDL_WINDOW_OPENGL;
#else
    ui32 flags = 0;
#endif // HEMLOCK_USING_OPENGL
    if (m_settings.is_fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (m_settings.is_borderless) {
        flags |= SDL_WINDOW_BORDERLESS;
    }
    if (m_settings.is_resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    m_window = SDL_CreateWindow(name().data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width(), height(), flags);
    if (m_window == nullptr) {
        debug_printf("Couldn't create SDL Window.\n");

        debug_printf(SDL_GetError());

        return WindowError::SDL_WINDOW;
    }

#ifdef HEMLOCK_USING_OPENGL
    m_context = SDL_GL_CreateContext(m_window);
    if (m_context == nullptr) {
        debug_printf("Couldn't create OpenGL context for SDL Window.\n");

        debug_printf(SDL_GetError());

        return WindowError::SDL_GL_CONTEXT;
    }

    GLenum error = glewInit();
    if (error != GLEW_OK) {
        debug_printf("Couldn't initialise Glew.\n");

        debug_printf(glewGetErrorString(error));

        return WindowError::GLEW_INIT;
    }

    // TODO(Matthew): Do we do this here? For multiple windows do these need resetting
    //                each time we change which window we are working on?
    {
        // Enable depth testing, set the clear colour and depth.
        glClearColor(0.2f, 0.7f, 0.3f, 1.0f);
        glClearDepth(1.0);

        // Enable blending.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

        if (m_settings.swap_interval == SwapInterval::V_SYNC) {
            SDL_GL_SetSwapInterval(1);
        } else {
            SDL_GL_SetSwapInterval(0);
        }
    }
#endif // HEMLOCK_USING_OPENGL

    m_window_id = SDL_GetWindowID(m_window);
#endif // HEMLOCK_USING_SDL

    hui::InputDispatcher::instance()->on_window.move   += &handle_external_window_move;
    hui::InputDispatcher::instance()->on_window.resize += &handle_external_window_resize;

    return WindowError::NONE;
}

void happ::Window::dispose() {
    if (!m_initialised) return;
    m_initialised = false;

    hui::InputDispatcher::instance()->on_window.move   -= &handle_external_window_move;
    hui::InputDispatcher::instance()->on_window.resize -= &handle_external_window_resize;

#ifdef HEMLOCK_USING_SDL
#ifdef HEMLOCK_USING_OPENGL
    SDL_GL_DeleteContext(m_context);
    m_context = nullptr;
#endif // HEMLOCK_USING_OPENGL

    SDL_DestroyWindow(m_window);
    m_window = nullptr;
#endif // HEMLOCK_USING_SDL

    WindowDimensionMap().swap(m_allowed_resolutions);
    FullscreenModeMap().swap(m_fullscreen_modes);
}

void happ::Window::set_name(const std::string& name) {
    m_settings.name = name;

#ifdef HEMLOCK_USING_SDL
    SDL_SetWindowTitle(m_window, name.data());
#endif // HEMLOCK_USING_SDL
}

void happ::Window::set_dimensions(WindowDimensions dimensions) {
    if (m_settings.dimensions == dimensions) return;

#ifdef HEMLOCK_USING_SDL
    SDL_SetWindowSize(m_window, dimensions.width, dimensions.height);

#ifdef HEMLOCK_USING_OPENGL
    // TODO(Matthew): do we need to call this for fullscreen too?
    // TODO(Matthew): do we need to call this each time we change which
    //                window we are handling?
    glViewport(0, 0, dimensions.width, dimensions.height);
#endif // HEMLOCK_USING_OPENGL
#endif // HEMLOCK_USING_SDL

    WindowDimensions temp = m_settings.dimensions;
    m_settings.dimensions = dimensions;

    calculate_aspect_ratio();

    on_window_resize(ResizeEvent{ temp, m_settings.dimensions });
}

void happ::Window::set_width(ui32 width) {
    set_dimensions({{ width, height() }});
}

void happ::Window::set_height(ui32 height) {
    set_dimensions({{ width(), height }});
}

void happ::Window::set_display(ui32 display_idx) {
    if (m_settings.display_idx == display_idx) return;

    m_settings.display_idx = display_idx;

    // We don't want to invoke any events for leaving fullscreen
    // as we are just temporarily going windowed to change display.
    if (m_settings.is_fullscreen) {
#ifdef HEMLOCK_USING_SDL
        SDL_SetWindowFullscreen(m_window, 0);
#endif // HEMLOCK_USING_SDL
    }

    // Make sure the new display allows the current dimensions,
    // setting new ones if not.
    validate_dimensions();

#ifdef HEMLOCK_USING_SDL
    // Centre window on new display.
    SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED_DISPLAY(display_idx), SDL_WINDOWPOS_CENTERED_DISPLAY(display_idx));
#endif // HEMLOCK_USING_SDL

    // Make sure the new display allows the current fullscreen
    // mode, setting a new one if not.
    validate_fullscreen_mode();

    // And change back to how we were!
    if (m_settings.is_fullscreen) {
#ifdef HEMLOCK_USING_SDL
        SDL_SetWindowFullscreen(
            m_window,
            m_settings.fake_fullscreen ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN
        );
#endif // HEMLOCK_USING_SDL
    }

    on_display_change();
}

void happ::Window::set_fullscreen_mode(FullscreenMode fullscreen_mode) {
    if (m_settings.fullscreen_mode == fullscreen_mode) return;

    FullscreenMode tmp = m_settings.fullscreen_mode;
    m_settings.fullscreen_mode = fullscreen_mode;

#ifdef HEMLOCK_USING_SDL
    SDL_DisplayMode mode = {
        fullscreen_mode.pixel_format,
        static_cast<int>(fullscreen_mode.resolution.width),
        static_cast<int>(fullscreen_mode.resolution.height),
        static_cast<int>(fullscreen_mode.refresh_rate),
        nullptr
    };
    if (!SDL_SetWindowDisplayMode(m_window, &mode)) {
        debug_printf("Could not set window display mode for %u.\n", m_window_id);

        debug_printf(SDL_GetError());
    }
#endif // HEMLOCK_USING_SDL

    if (m_settings.is_fullscreen) {
        FullscreenModeChangeEvent fmce{ tmp, fullscreen_mode };
        on_fullscreen_mode_change(fmce);
    }

    if (tmp.resolution != fullscreen_mode.resolution) {
        ResizeEvent re{ tmp.resolution, fullscreen_mode.resolution };
        on_window_resize(re);
    }
}

void happ::Window::set_fake_fullscreen(bool fake_fullscreen) {
    if (m_settings.fake_fullscreen == fake_fullscreen) return;

    m_settings.fake_fullscreen = fake_fullscreen;

    if (m_settings.is_fullscreen) {
#ifdef HEMLOCK_USING_SDL
        SDL_SetWindowFullscreen(
            m_window,
            m_settings.fake_fullscreen ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN
        );
#endif // HEMLOCK_USING_SDL
    }
}

void happ::Window::set_is_fullscreen(bool fullscreen) {
    if (m_settings.is_fullscreen == fullscreen) return;

    m_settings.is_fullscreen = fullscreen;

    if (fullscreen) {
#ifdef HEMLOCK_USING_SDL
        SDL_SetWindowFullscreen(
            m_window,
            m_settings.fake_fullscreen ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN
        );
#endif // HEMLOCK_USING_SDL
        on_window_fullscreen_enter();
    } else {
#ifdef HEMLOCK_USING_SDL
        SDL_SetWindowFullscreen(m_window, 0);
#endif // HEMLOCK_USING_SDL
        on_window_fullscreen_exit();
    }
}

void happ::Window::set_is_resizable(bool resizable) {
    if (m_settings.is_resizable == resizable) return;

    m_settings.is_resizable = resizable;

#ifdef HEMLOCK_USING_SDL
    SDL_SetWindowResizable(m_window, static_cast<SDL_bool>(resizable));
#endif // HEMLOCK_USING_SDL
}

void happ::Window::set_is_borderless(bool borderless) {
    if (m_settings.is_borderless == borderless) return;

    if (m_settings.is_fullscreen && borderless) {
        set_is_fullscreen(false);
    }

    m_settings.is_borderless = borderless;

#ifdef HEMLOCK_USING_SDL
    SDL_SetWindowBordered(m_window, static_cast<SDL_bool>(!borderless));
#endif // HEMLOCK_USING_SDL
}

void happ::Window::set_is_maximised(bool maximised) {
    if (m_settings.is_maximised == maximised) return;
    m_settings.is_maximised = maximised;
    if (maximised) {
#ifdef HEMLOCK_USING_SDL
        SDL_MaximizeWindow(m_window);
#endif // HEMLOCK_USING_SDL
        on_window_maximise();
    } else {
#ifdef HEMLOCK_USING_SDL
        SDL_MinimizeWindow(m_window);
#endif // HEMLOCK_USING_SDL
        on_window_minimise();
    }
}

void happ::Window::set_swap_interval(SwapInterval swap_interval) {
    if (m_settings.swap_interval == swap_interval) return;

    m_settings.swap_interval = swap_interval;

    int vsync = (swap_interval == SwapInterval::V_SYNC ? 1 : 0);
#ifdef HEMLOCK_USING_SDL
    SDL_GL_SetSwapInterval(vsync);
#endif // HEMLOCK_USING_SDL
}

void happ::Window::set_allowed_resolutions(WindowDimensionMap allowed_resolutions) {
    m_allowed_resolutions = allowed_resolutions;

    // If we're windowed and not resizable, we must keep window to one of the allowed resolutions.
    if (!m_settings.is_resizable && !m_settings.is_maximised && !m_settings.is_fullscreen) {
        validate_dimensions();
    }
}

void happ::Window::sync() {
#ifdef HEMLOCK_USING_SDL
#ifdef HEMLOCK_USING_OPENGL
    SDL_GL_SwapWindow(m_window);
#endif // HEMLOCK_USING_OPENGL
#endif // HEMLOCK_USING_SDL
}

void happ::Window::check_display_occupied() {
    ui32 current_display_idx = static_cast<ui32>(SDL_GetWindowDisplayIndex(m_window));
    if (m_settings.display_idx == current_display_idx) return;

    m_settings.display_idx = current_display_idx;

    on_display_change();
}

void happ::Window::determine_modes() {
#ifdef HEMLOCK_USING_SDL
    // First get number of displays attached to the machine.
    ui32 display_count = SDL_GetNumVideoDisplays();
    for (ui32 display_idx = 0; display_idx < display_count; ++display_idx) {
        // Now get number of modes each given display can operate in.
        ui32 mode_count = SDL_GetNumDisplayModes(display_idx);

        // Prepare buffers for display we are currently looking at.
        //   Note: we don't know how many unique resolutions we'll end up allowing,
        //   so we don't emplace it.
        m_fullscreen_modes[display_idx] = std::vector<FullscreenMode>(mode_count);
        std::vector<WindowDimensions> allowed_resolutions;

        for (ui32 mode_idx = 0; mode_idx < mode_count; ++mode_idx) {
            SDL_DisplayMode mode;
            SDL_GetDisplayMode(display_idx, mode_idx, &mode);

            WindowDimensions resolution{{ static_cast<ui32>(mode.w), static_cast<ui32>(mode.h) }};

            m_fullscreen_modes[display_idx][mode_idx] = FullscreenMode{
                resolution, static_cast<ui32>(mode.refresh_rate), mode.format
            };

            // Can just check last allowed resolution as SDL orders modes
            // by resolution first.
            if (mode_idx == 0 || allowed_resolutions.back() != resolution) {
                allowed_resolutions.push_back(resolution);
            }
        }

        // Finally set the allowed resolutions.
        m_allowed_resolutions[display_idx] = allowed_resolutions;
    }
#endif // HEMLOCK_USING_SDL
}

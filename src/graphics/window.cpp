#include "stdafx.h"

#include "graphics/window.h"

bool hg::WindowDimensions::operator==(const WindowDimensions& rhs) {
    return this->width == rhs.width && this->height == rhs.height;
}

bool hg::WindowDimensions::operator!=(const WindowDimensions& rhs) {
    return !(this->width == rhs.width && this->height == rhs.height);
}

bool hg::FullscreenMode::operator==(const FullscreenMode& rhs) {
    return this->resolution == rhs.resolution
                && this->refresh_rate == rhs.refresh_rate
                && this->pixel_format == rhs.pixel_format;
}

bool hg::FullscreenMode::operator!=(const FullscreenMode& rhs) {
    return !(this->resolution == rhs.resolution
                && this->refresh_rate == rhs.refresh_rate
                && this->pixel_format == rhs.pixel_format);
}

hg::Window::Window(WindowSettings settings) :
    m_initialised(false),
    m_window(nullptr),
    m_settings(settings),
    m_aspect_ratio({})
{ /* Empty. */ }

hg::WindowError hg::Window::init() {
    if (m_initialised) return WindowError::NONE;
    m_initialised = true;

    determine_modes();
    calculate_aspect_ratio();

    ui32 flags = SDL_WINDOW_OPENGL;
    if (m_settings.is_fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (m_settings.is_borderless) {
        flags |= SDL_WINDOW_BORDERLESS;
    }
    if (m_settings.is_resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    m_window = SDL_CreateWindow(name(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width(), height(), flags);
    if (m_window == nullptr) {
        debug_printf("Couldn't create SDL Window.");
        return WindowError::SDL_WINDOW;
    }

    m_context = SDL_GL_CreateContext(m_window);
    if (m_context == NULL) {
        debug_printf("Couldn't create OpenGL context for SDL Window.");
        return WindowError::SDL_GL_CONTEXT;
    }

    GLenum error = glewInit();
    if (error != GLEW_OK) {
        debug_printf("Couldn't initialise Glew.");
        return WindowError::GLEW_INITIALISATION;
    }

    printf("*** OpenGL Version:  %s ***\n", glGetString(GL_VERSION));
    printf("*** OpenGL Renderer: %s ***\n", glGetString(GL_RENDERER));

    GLint nr_attributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nr_attributes);
    debug_printf("Maximum # of vertex attributes supported: %d.\n", nr_attributes);

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

    if (m_settings.swap_interval == SwapInterval::V_SYNC) {
        SDL_GL_SetSwapInterval(1);
    } else {
        SDL_GL_SetSwapInterval(0);
    }

    return WindowError::NONE;
}

void hg::Window::dispose() {
    if (!m_initialised) return;
    m_initialised = false;

    SDL_GL_DeleteContext(m_context);
    m_context = nullptr;

    SDL_DestroyWindow(m_window);
    m_window = nullptr;

    delete m_settings.name;
    m_settings.name = nullptr;

    WindowDimensionMap().swap(m_allowed_resolutions);
}

void hg::Window::set_name(char* name) {
    m_settings.name = name;
    SDL_SetWindowTitle(m_window, name);
}

void hg::Window::set_dimensions(WindowDimensions dimensions) {
    if (m_settings.dimensions == dimensions) return;

    SDL_SetWindowSize(m_window, dimensions.width, dimensions.height);

    // TODO(Matthew): do we need to call this for fullscreen too?
    glViewport(0, 0, dimensions.width, dimensions.height);

    WindowDimensions temp = m_settings.dimensions;
    m_settings.dimensions = dimensions;

    calculate_aspect_ratio();

    on_window_resize(ResizeEvent{ temp, m_settings.dimensions });
}

void hg::Window::set_width(ui32 width) {
    set_dimensions({ width, height() });
}

void hg::Window::set_height(ui32 height) {
    set_dimensions({ width(), height });
}

void hg::Window::set_display(ui32 display_idx) {
    if (m_settings.display_idx == display_idx) return;

    m_settings.display_idx = display_idx;

    // We don't want to invoke any events for leaving fullscreen
    // as we are just temporarily going windowed to change display.
    if (m_settings.is_fullscreen) {
        SDL_SetWindowFullscreen(m_window, 0);
    }

    // Make sure the new display allows the current dimensions,
    // setting new ones if not.
    validate_dimensions();

    // Centre window on new display.
    SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED_DISPLAY(display_idx), SDL_WINDOWPOS_CENTERED_DISPLAY(display_idx));

    // Make sure the new display allows the current fullscreen
    // mode, setting a new one if not.
    validate_fullscreen_mode();

    // And change back to how we were!
    if (m_settings.is_fullscreen) {
        SDL_SetWindowFullscreen(
            m_window,
            m_settings.fake_fullscreen ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN
        );
    }

    on_display_change();
}

void hg::Window::set_fullscreen_mode(FullscreenMode fullscreen_mode) {
    if (m_settings.fullscreen_mode == fullscreen_mode) return;

    FullscreenMode tmp = m_settings.fullscreen_mode;
    m_settings.fullscreen_mode = fullscreen_mode;

    if (m_settings.is_fullscreen) {
        FullscreenModeChangeEvent fmce{ tmp, fullscreen_mode };
        on_fullscreen_mode_change(fmce);
    }

    if (tmp.resolution != fullscreen_mode.resolution) {
        ResizeEvent re{ tmp.resolution, fullscreen_mode.resolution };
        on_window_resize(re);
    }
}

void hg::Window::set_fake_fullscreen(bool fake_fullscreen) {
    if (m_settings.fake_fullscreen == fake_fullscreen) return;

    m_settings.fake_fullscreen = fake_fullscreen;

    if (m_settings.is_fullscreen) {
        SDL_SetWindowFullscreen(
            m_window,
            m_settings.fake_fullscreen ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN
        );
    }
}

void hg::Window::set_is_fullscreen(bool fullscreen) {
    if (m_settings.is_fullscreen == fullscreen) return;

    m_settings.is_fullscreen = fullscreen;

    if (fullscreen) {
        SDL_SetWindowFullscreen(
            m_window,
            m_settings.fake_fullscreen ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN
        );
        on_window_fullscreen_enter();
    } else {
        SDL_SetWindowFullscreen(m_window, 0);
        on_window_fullscreen_exit();
    }
}

void hg::Window::set_is_resizable(bool resizable) {
    if (m_settings.is_resizable == resizable) return;

    m_settings.is_resizable = resizable;

    SDL_SetWindowResizable(m_window, (SDL_bool)resizable);
}

void hg::Window::set_is_borderless(bool borderless) {
    if (m_settings.is_borderless == borderless) return;

    if (m_settings.is_fullscreen && borderless) {
        set_is_fullscreen(false);
    }

    m_settings.is_borderless = borderless;

    SDL_SetWindowBordered(m_window, (SDL_bool)!borderless);
}

void hg::Window::set_is_maximised(bool maximised) {
    if (m_settings.is_maximised == maximised) return;
    m_settings.is_maximised = maximised;
    if (maximised) {
        SDL_MaximizeWindow(m_window);
        on_window_maximise();
    } else {
        SDL_MinimizeWindow(m_window);
        on_window_minimise();
    }
}

void hg::Window::set_swap_interval(SwapInterval swap_interval) {
    if (m_settings.swap_interval == swap_interval) return;

    m_settings.swap_interval = swap_interval;

    int vsync = (swap_interval == SwapInterval::V_SYNC ? 1 : 0);
    SDL_GL_SetSwapInterval(vsync);
}

void hg::Window::set_allowed_resolutions(WindowDimensionMap allowed_resolutions) {
    m_allowed_resolutions = allowed_resolutions;

    // If we're windowed and not resizable, we must keep window to one of the allowed resolutions.
    if (!m_settings.is_resizable&& !m_settings.is_maximised && !m_settings.is_fullscreen) {
        validate_dimensions();
    }
}

void hg::Window::check_display_occupied() {
    ui32 current_display_idx = (ui32)SDL_GetWindowDisplayIndex(m_window);
    if (m_settings.display_idx == current_display_idx) return;

    m_settings.display_idx = current_display_idx;

    on_display_change();
}

void hg::Window::determine_modes() {
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

            WindowDimensions resolution{ (ui32)mode.w, (ui32)mode.h };

            m_fullscreen_modes[display_idx][mode_idx] = FullscreenMode{
                resolution, (ui32)mode.refresh_rate, (ui32)mode.format
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
}

void hg::Window::calculate_aspect_ratio() {
    std::function<ui32(ui32, ui32)> calculateGCD = [&calculateGCD](ui32 x, ui32 y) {
        if (y > x) return calculateGCD(y, x);
        if (y == 0) return x;

        return calculateGCD(y, x % y);
    };

    ui32 width = this->width(), height = this->height();
    ui32 gcd = calculateGCD(width, height);

    m_aspect_ratio = { width/gcd, height/gcd };
}

void hg::Window::validate_dimensions() {
    // Track closest allowed dimension.
    WindowDimensions closest_dimensions{m_settings.dimensions};
    ui32 distance2 = std::numeric_limits<ui32>::max();

    // Check each allowed resolution on the display currently occupied by the window.
    bool current_dimensions_allowed = false;
    for (auto& resolution : m_allowed_resolutions[m_settings.display_idx]) {
        if (m_settings.dimensions == resolution) {
            current_dimensions_allowed = true;
            break;
        }

        // Is this resolution "closer" to the current dimensions of the window?
        ui32 new_distance2 = std::pow(closest_dimensions.width - resolution.width, 2)
                                + std::pow(closest_dimensions.height - resolution.height, 2);
        if (new_distance2 < distance2) {
            closest_dimensions = resolution;
            distance2 = new_distance2;
        }
    }

    // If the current dimensions aren't allowed, change to
    // the dimensions we found to be the closest allowed.
    if (!current_dimensions_allowed) {
        set_dimensions(closest_dimensions);
    }
}

void hg::Window::validate_fullscreen_mode() {
    // Check the current fullscreen mode is allowed on the new display.
    // If not change the mode to a reasonable alternative.
    auto it = std::find(
        m_fullscreen_modes[m_settings.display_idx].begin(),
        m_fullscreen_modes[m_settings.display_idx].end(),
        m_settings.fullscreen_mode
    );
    if (it == m_fullscreen_modes[m_settings.display_idx].end()) {
        /***********************************************************\
         * Test for a fullscreen mode with given resolution width. *
         * If found, set that as the new fullscreen mode.          *
        \***********************************************************/
        auto try_res_width = [&](ui32 width) -> bool {
            auto it = std::find_if(
                m_fullscreen_modes[m_settings.display_idx].begin(),
                m_fullscreen_modes[m_settings.display_idx].end(),
                [width](const FullscreenMode& mode) {
                    return mode.resolution.width == width;
                }
            );
            if (it != m_fullscreen_modes[m_settings.display_idx].end()) {
                m_settings.fullscreen_mode = *it;
                return true;
            }
            return false;
        };

        // Do the tests and worst case fallback to the first mode SDL supports.
        if (!try_res_width(1920) && !try_res_width(1280) && !try_res_width(800)) {
            m_settings.fullscreen_mode = m_fullscreen_modes[m_settings.display_idx][0];
        }
    }
}

void hg::Window::sync() {
    SDL_GL_SwapWindow(m_window);
}

#include "stdafx.h"

#include "ui/input/dispatcher.h"

#include "app/window/window.h"

happ::Window::Window() :
    WindowBase(),
    m_window(nullptr),
#if defined(HEMLOCK_USING_OPENGL)
    m_context(nullptr)
#elif defined(HEMLOCK_USING_VULKAN) // HEMLOCK_USING_OPENGL
    m_evalutor(DeviceEvaluator{
        [&](VkPhyiscalDevice device) {
            // TODO(Matthew): Implement.
        };
    });
    m_instance(nullptr),
    m_device(nullptr),
    m_surface(nullptr),
    m_extensions({0, nullptr}),
    m_available_devices({0, nullptr})
#endif // HEMLOCK_USING_VULKAN
{ /* Empty. */ }

happ::WindowError happ::Window::init(WindowSettings settings /*= {}*/) {
    if (m_initialised) return WindowError::NONE;
    m_initialised = true;

    m_settings = settings;

    determine_modes();

    ui32 flags = 0;

    // TODO(Matthew): Extract SDL and none-SDL parts of below as some things
    //                in OpenGL & Vulkan sections are generic even if we were
    //                to support some other library like GLFW.
#if defined(HEMLOCK_USING_SDL)
#if defined(HEMLOCK_USING_OPENGL)
    flags |= SDL_WINDOW_OPENGL;
#elif defined(HEMLOCK_USING_VULKAN) // defined(HEMLOCK_USING_OPENGL)
    flags |= SDL_WINDOW_VULKAN;
#endif // defined(HEMLOCK_USING_VULKAN)
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

        debug_printf("%s\n", SDL_GetError());

        return WindowError::SDL_WINDOW;
    }

#if defined(HEMLOCK_USING_OPENGL)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    m_context = SDL_GL_CreateContext(m_window);
    if (m_context == nullptr) {
        debug_printf("Couldn't create OpenGL context for SDL Window.\n");

        debug_printf("%s\n", SDL_GetError());

        return WindowError::SDL_GL_CONTEXT;
    }

    GLenum error = glewInit();
    if (error != GLEW_OK) {
        debug_printf("Couldn't initialise Glew.\n");

        debug_printf(
            "%s\n",
            reinterpret_cast<const char*>(glewGetErrorString(error))
        );

        return WindowError::GLEW_INIT;
    }

    // TODO(Matthew): Do we do this here? For multiple windows do these need resetting
    //                each time we change which window we are working on?
    //                    Definitely don't as we would like to, e.g. not necessarily
    //                    cull free-floating UI elements that use sprite batcher.
    {
        // Enable depth testing, set the clear colour and depth.
        glClearColor(0.2f, 0.7f, 0.3f, 1.0f);
        glClearDepth(1.0);

        // Enable blending.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

        // Enable depth testing.
        glEnable(GL_DEPTH_TEST);

        // Enable face culling.
        glEnable(GL_CULL_FACE);
        // glFrontFace(GL_CW);

        if (m_settings.swap_interval == SwapInterval::V_SYNC) {
            SDL_GL_SetSwapInterval(1);
        } else {
            SDL_GL_SetSwapInterval(0);
        }
    }
#elif defined(HEMLOCK_USING_VULKAN) // defined(HEMLOCK_USING_OPENGL)
#if defined(DEBUG)
    {
        ui32 total_extension_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &total_extension_count, nullptr);
        VkExtensionProperties* extensions = new VkExtensionProperties[total_extension_count];
        vkEnumerateInstanceExtensionProperties(nullptr, &total_extension_count, extensions);

        debug_printf("Available Vulkan Extensions:\n");
        for (ui32 i = 0; i < total_extension_count; ++i) {
            debug_printf("    -> %s - spec v%d\n", extensions[i].extensionName, extensions[i].specVersion);
        }

        ui32 total_layer_count = 0;
        vkEnumerateInstanceLayerProperties(nullptr, &total_layer_count, nullptr);
        VkLayerProperties* layers = new VkLayerProperties[total_layer_count];
        vkEnumerateInstanceLayerProperties(nullptr, &total_layer_count, layers);

        debug_printf("Available Vulkan Validation Layers:\n");
        for (ui32 i = 0; i < total_layer_count; ++i) {
            debug_printf("    -> %s - impl v%d - spec v%d\n", layers[i].layerName, layers[i].implementationVersion, layers[i].specVersion);
            debug_printf("          - %s\n", layers[i].description);
        }
    }
#endif // defined(DEBUG)

    // Get number of Vulkan extensions required to create our instance
    // and surface for rendering with Vulkan to our window.
    if (!SDL_Vulkan_GetInstanceExtensions(m_window, &m_extensions.count, nullptr)) {
        debug_printf("Could not get count of extensions needed to create a Vulkan instance useable with SDL.\n");

        return WindowError::VULKAN_INSTANCE;
    }
    // Allocate that number of pointer slots for inserting names to.
    m_extensions.names = new const char*[m_extensions.count];

    // Get the names of those extensions.
    if (!SDL_Vulkan_GetInstanceExtensions(m_window, &m_extensions.count, m_extensions.names)) {
        debug_printf("Could not get extensions needed to create a Vulkan instance useable with SDL.\n");

        return WindowError::VULKAN_INSTANCE;
    }

    // If DEBUG is set, this will create a constant list of validation layer names
    // that vulkan should use to give us feedback on things going wrong.
    CREATE_VK_VALIDATION_LAYERS;

    VkInstanceCreateInfo instance_info = {
        .sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                      = nullptr,
        .flags                      = 0,
        .pApplicationInfo           = &APP_INFO,
        .enabledLayerCount          = ENABLE_VK_VALIDATION_LAYERS,
        .ppEnabledLayerNames        = VK_VALIDATION_LAYERS,
        .enabledExtensionCount      = m_extensions.count,
        .ppEnabledExtensionNames    = m_extensions.names
    };

    VkResult err = vkCreateInstance(&instance_info, nullptr, &m_instance);
    if (err != VkResult::VK_SUCCESS) {
        debug_printf("Could not create Vulkan istance.\n");

        return WindowError::VULKAN_INSTANCE;
    }

    if (!determine_devices()) {
        debug_printf("No physical devices availabile with support for Vulkan.\n");

        return WindowError::VULKAN_HARDWARE;
    }

    SDL_Vulkan_CreateSurface(m_window, m_instance, m_surface);
    if (m_instance == nullptr) {
        debug_printf("Couldn't create Vulkan instance for SDL Window.\n");

        debug_printf("%s\n", SDL_GetError());

        return WindowError::SDL_VULKAN_SURFACE;
    }
#endif // defined(HEMLOCK_USING_VULKAN)

    m_window_id = SDL_GetWindowID(m_window);
#endif // defined(HEMLOCK_USING_SDL)

    hui::InputDispatcher::instance()->on_window.move   += &handle_external_window_move;
    hui::InputDispatcher::instance()->on_window.resize += &handle_external_window_resize;

    return WindowError::NONE;
}

void happ::Window::dispose() {
    if (!m_initialised) return;
    m_initialised = false;

    hui::InputDispatcher::instance()->on_window.move   -= &handle_external_window_move;
    hui::InputDispatcher::instance()->on_window.resize -= &handle_external_window_resize;

#if defined(HEMLOCK_USING_SDL)
#if defined(HEMLOCK_USING_OPENGL)
    SDL_GL_DeleteContext(m_context);
    m_context = nullptr;
#elif defined(HEMLOCK_USING_VULKAN) // defined(HEMLOCK_USING_OPENGL)
    vkDestroyInstance(m_instance, nullptr);
    m_instance = nullptr;
#endif // defined(HEMLOCK_USING_VULKAN)

    SDL_DestroyWindow(m_window);
    m_window = nullptr;
#endif // defined(HEMLOCK_USING_SDL)

    WindowDimensionMap().swap(m_allowed_resolutions);
    FullscreenModeMap().swap(m_fullscreen_modes);
}

void happ::Window::set_name(const std::string& name) {
    m_settings.name = name;

#if defined(HEMLOCK_USING_SDL)
    SDL_SetWindowTitle(m_window, name.data());
#endif // defined(HEMLOCK_USING_SDL)
}

void happ::Window::set_dimensions(WindowDimensions dimensions) {
    if (m_settings.dimensions == dimensions) return;

#if defined(HEMLOCK_USING_SDL)
    SDL_SetWindowSize(m_window, dimensions.width, dimensions.height);

#if defined(HEMLOCK_USING_OPENGL)
    // TODO(Matthew): do we need to call this for fullscreen too?
    // TODO(Matthew): do we need to call this each time we change which
    //                window we are handling?
    glViewport(0, 0, dimensions.width, dimensions.height);
#endif // defined(HEMLOCK_USING_OPENGL)
#endif // defined(HEMLOCK_USING_SDL)

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
#if defined(HEMLOCK_USING_SDL)
        SDL_SetWindowFullscreen(m_window, 0);
#endif // defined(HEMLOCK_USING_SDL)
    }

    // Make sure the new display allows the current dimensions,
    // setting new ones if not.
    validate_dimensions();

#if defined(HEMLOCK_USING_SDL)
    // Centre window on new display.
    SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED_DISPLAY(display_idx), SDL_WINDOWPOS_CENTERED_DISPLAY(display_idx));
#endif // defined(HEMLOCK_USING_SDL)

    // Make sure the new display allows the current fullscreen
    // mode, setting a new one if not.
    validate_fullscreen_mode();

    // And change back to how we were!
    if (m_settings.is_fullscreen) {
#if defined(HEMLOCK_USING_SDL)
        SDL_SetWindowFullscreen(
            m_window,
            m_settings.fake_fullscreen ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN
        );
#endif // defined(HEMLOCK_USING_SDL)
    }

    on_display_change();
}

void happ::Window::set_fullscreen_mode(FullscreenMode fullscreen_mode) {
    if (m_settings.fullscreen_mode == fullscreen_mode) return;

    FullscreenMode tmp = m_settings.fullscreen_mode;
    m_settings.fullscreen_mode = fullscreen_mode;

#if defined(HEMLOCK_USING_SDL)
    SDL_DisplayMode mode = {
        fullscreen_mode.pixel_format,
        static_cast<int>(fullscreen_mode.resolution.width),
        static_cast<int>(fullscreen_mode.resolution.height),
        static_cast<int>(fullscreen_mode.refresh_rate),
        nullptr
    };
    if (!SDL_SetWindowDisplayMode(m_window, &mode)) {
        debug_printf("Could not set window display mode for %u.\n", m_window_id);

        debug_printf("%s\n", SDL_GetError());
    }
#endif // defined(HEMLOCK_USING_SDL)

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
#if defined(HEMLOCK_USING_SDL)
        SDL_SetWindowFullscreen(
            m_window,
            m_settings.fake_fullscreen ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN
        );
#endif // defined(HEMLOCK_USING_SDL)
    }
}

void happ::Window::set_is_fullscreen(bool fullscreen) {
    if (m_settings.is_fullscreen == fullscreen) return;

    m_settings.is_fullscreen = fullscreen;

    if (fullscreen) {
#if defined(HEMLOCK_USING_SDL)
        SDL_SetWindowFullscreen(
            m_window,
            m_settings.fake_fullscreen ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN
        );
#endif // defined(HEMLOCK_USING_SDL)
        on_window_fullscreen_enter();
    } else {
#if defined(HEMLOCK_USING_SDL)
        SDL_SetWindowFullscreen(m_window, 0);
#endif // defined(HEMLOCK_USING_SDL)
        on_window_fullscreen_exit();
    }
}

void happ::Window::set_is_resizable(bool resizable) {
    if (m_settings.is_resizable == resizable) return;

    m_settings.is_resizable = resizable;

#if defined(HEMLOCK_USING_SDL)
    SDL_SetWindowResizable(m_window, static_cast<SDL_bool>(resizable));
#endif // defined(HEMLOCK_USING_SDL)
}

void happ::Window::set_is_borderless(bool borderless) {
    if (m_settings.is_borderless == borderless) return;

    if (m_settings.is_fullscreen && borderless) {
        set_is_fullscreen(false);
    }

    m_settings.is_borderless = borderless;

#if defined(HEMLOCK_USING_SDL)
    SDL_SetWindowBordered(m_window, static_cast<SDL_bool>(!borderless));
#endif // defined(HEMLOCK_USING_SDL)
}

void happ::Window::set_is_maximised(bool maximised) {
    if (m_settings.is_maximised == maximised) return;
    m_settings.is_maximised = maximised;
    if (maximised) {
#if defined(HEMLOCK_USING_SDL)
        SDL_MaximizeWindow(m_window);
#endif // defined(HEMLOCK_USING_SDL)
        on_window_maximise();
    } else {
#if defined(HEMLOCK_USING_SDL)
        SDL_MinimizeWindow(m_window);
#endif // defined(HEMLOCK_USING_SDL)
        on_window_minimise();
    }
}

void happ::Window::set_swap_interval(SwapInterval swap_interval) {
    if (m_settings.swap_interval == swap_interval) return;

    m_settings.swap_interval = swap_interval;

    int vsync = (swap_interval == SwapInterval::V_SYNC ? 1 : 0);
#if defined(HEMLOCK_USING_SDL)
    SDL_GL_SetSwapInterval(vsync);
#endif // defined(HEMLOCK_USING_SDL)
}

void happ::Window::set_allowed_resolutions(WindowDimensionMap allowed_resolutions) {
    m_allowed_resolutions = allowed_resolutions;

    // If we're windowed and not resizable, we must keep window to one of the allowed resolutions.
    if (!m_settings.is_resizable && !m_settings.is_maximised && !m_settings.is_fullscreen) {
        validate_dimensions();
    }
}

void happ::Window::sync() {
#if defined(HEMLOCK_USING_SDL)
#if defined(HEMLOCK_USING_OPENGL)
    SDL_GL_SwapWindow(m_window);
#endif // defined(HEMLOCK_USING_OPENGL)
#endif // defined(HEMLOCK_USING_SDL)
}

#if defined(HEMLOCK_USING_VULKAN)
void happ::Window::set_device_evaluator(DeviceEvaluator&& evaluator) {
    m_evaluator = evaluator;
}
#endif // defined(HEMLOCK_USING_VULKAN)

void happ::Window::check_display_occupied() {
    ui32 current_display_idx = static_cast<ui32>(SDL_GetWindowDisplayIndex(m_window));
    if (m_settings.display_idx == current_display_idx) return;

    m_settings.display_idx = current_display_idx;

    on_display_change();
}

void happ::Window::determine_modes() {
#if defined(HEMLOCK_USING_SDL)
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
#endif // defined(HEMLOCK_USING_SDL)
}

#if defined(HEMLOCK_USING_VULKAN)
bool happ::Window::determine_devices() {
    ui32 device_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
    if (device_count == 0) return false;
    VkPhysicalDevice* candidate_devices = new VkPhyscalDevice[device_count];

    // Build up a list of the indices of the candidate devices
    // that pass the scoring, sorted by their score such that
    // the highest scoring device is at the end of the list.
    std::multimap<i32, ui32> valid_devices;
    for (ui32 i = 0; i < device_count; ++i) {
        i32 score = m_evaluator(candidate_devices[i]);
        if (score > 0) {
            valid_devices.insert({score, i});
        }
    }

    if (valid_devices.empty()) return false;

    // Add valid devices to our available devices list.
    m_available_devices.count   = valid_devices.size();
    m_available_devices.devices = new VkPhysicalDevice[m_available_devices.count];
    ui32 i = 0;
    for (auto valid_device : valid_devices) {
        m_available_devices.devices[i++] = candidate_devices[valid_device.second];
    }

    // Set top-scoring device as the device we will use by default.
    m_device = m_available_devices.devices[m_available_devices.count - 1];

    return true;
}
#endif // defined(HEMLOCK_USING_VULKAN)

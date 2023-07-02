#ifndef __hemlock_app_window_state_hpp
#define __hemlock_app_window_state_hpp

namespace hemlock {
    namespace app {
        /**
         * @brief Enumerates errors that may
         * occur while initialising a window.
         */
        enum class WindowError {
            NONE       = 0,
            SDL_WINDOW = -1,
#if defined(HEMLOCK_USING_OPENGL)
            SDL_GL_CONTEXT = -2,
            GLEW_INIT      = -3
#elif defined(HEMLOCK_USING_VULKAN)  // defined(HEMLOCK_USING_OPENGL)
            VULKAN_INSTANCE    = -2,
            VULKAN_HARDWARE    = -3,
            SDL_VULKAN_SURFACE = -4
#endif                               // defined(HEMLOCK_USING_VULKAN)
        };

        /**
         * @brief Syntactic sugar on dimensions.
         */
        union WindowDimensions {
            ui32v2 data;

            struct {
                ui32 width, height;
            };
        };

        using WindowDimensionMap = std::map<ui8, std::vector<WindowDimensions>>;

        /**
         * @brief Holds settings for a fullscreen mode.
         */
        struct FullscreenMode {
            WindowDimensions resolution;
            ui32             refresh_rate;
            ui32             pixel_format;
        };

        using FullscreenModeMap = std::map<ui8, std::vector<FullscreenMode>>;

        /**
         * @brief Possible swap interval settings.
         */
        enum class SwapInterval {
            UNCAPPED = 0,
            V_SYNC   = 1,
            CAPPED   = 2
        };

        /**
         * @brief Basic settings for a window.
         */
        struct WindowSettings {
            std::string      name       = "Hemlock Window";
            WindowDimensions dimensions = {
                {800, 600}
            };
            ui32           display_idx     = 0;
            FullscreenMode fullscreen_mode = {};
            bool           fake_fullscreen = false;
            bool           is_fullscreen   = false;
            bool           is_resizable    = true;
            bool           is_borderless   = false;
            bool           is_maximised    = false;
            SwapInterval   swap_interval   = SwapInterval::V_SYNC;
        };

        struct ResizeEvent {
            WindowDimensions before, now;
        };

        struct FullscreenModeChangeEvent {
            FullscreenMode before, now;
        };
    }  // namespace app
}  // namespace hemlock
namespace happ = hemlock::app;

bool operator==(const happ::WindowDimensions& lhs, const happ::WindowDimensions& rhs);
bool operator!=(const happ::WindowDimensions& lhs, const happ::WindowDimensions& rhs);

bool operator==(const happ::FullscreenMode& lhs, const happ::FullscreenMode& rhs);
bool operator!=(const happ::FullscreenMode& lhs, const happ::FullscreenMode& rhs);

#endif  // __hemlock_app_window_state_hpp

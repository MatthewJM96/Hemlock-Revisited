#ifndef __hemlock_graphics_window_state_hpp
#define __hemlock_graphics_window_state_hpp

namespace hemlock {
    namespace graphics {
        /**
         * @brief Enumerates errors that may
         * occur while initialising a window.
         */
        enum class WindowError {
            NONE                =  0,
            SDL_WINDOW          = -1,
            SDL_GL_CONTEXT      = -2,
            GLEW_INIT           = -3
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
        using FullscreenModeMap  = std::map<ui8, std::vector<FullscreenMode>>;

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
            std::string      name            = "Hemlock Window";
            WindowDimensions dimensions      = {{800, 600}};
            ui32             display_idx     = 0;
            FullscreenMode   fullscreen_mode = {};
            bool             fake_fullscreen = false;
            bool             is_fullscreen   = false;
            bool             is_resizable    = true;
            bool             is_borderless   = false;
            bool             is_maximised    = false;
            SwapInterval     swap_interval   = SwapInterval::V_SYNC;
        };

        struct ResizeEvent {
            WindowDimensions before, now;
        };

        struct FullscreenModeChangeEvent {
            FullscreenMode before, now;
        };
    }
}
namespace hg = hemlock::graphics;

bool operator==(const hg::WindowDimensions& lhs, const hg::WindowDimensions& rhs);
bool operator!=(const hg::WindowDimensions& lhs, const hg::WindowDimensions& rhs);

bool operator==(const hg::FullscreenMode& lhs, const hg::FullscreenMode& rhs);
bool operator!=(const hg::FullscreenMode& lhs, const hg::FullscreenMode& rhs);

#endif // __hemlock_graphics_window_state_hpp

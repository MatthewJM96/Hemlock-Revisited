#ifndef __hemlock_graphics_window_h
#define __hemlock_graphics_window_h

namespace hemlock {
    namespace ui {
        class InputDispatcher;
    }

    namespace graphics {
        // Events:
        //     on_display_change
        //     on_window_close
        //     on_window_minimise
        //     on_window_maximise
        //     on_window_fullscreen_enter
        //     on_window_fullscreen_exit
        //     on_window_resize
        //     on_fullscreen_mode_change

        enum class WindowError {
            NONE = 1,
            SDL_WINDOW = -1,
            SDL_GL_CONTEXT = -2,
            GLEW_INITIALISATION = -3
        };

        struct WindowDimensions {
            ui32 width, height;

            bool operator==(const WindowDimensions& rhs);
            bool operator!=(const WindowDimensions& rhs);
        };

        struct FullscreenMode {
            WindowDimensions resolution;
            ui32             refresh_rate;
            ui32             pixel_format;

            bool operator==(const FullscreenMode& rhs);
            bool operator!=(const FullscreenMode& rhs);
        };

        enum class SwapInterval {
            UNCAPPED = 0,
            V_SYNC = 1,
            CAPPED = 2
        };

        struct WindowSettings {
            CALLEE_DELETE
            const char*      name            = "Hemlock Window";
            WindowDimensions dimensions      = {800, 600};
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
            WindowDimensions before;
            WindowDimensions now;
        };

        struct FullscreenModeChangeEvent {
            FullscreenMode before;
            FullscreenMode now;
        };

        class Window {
            friend class hemlock::ui::InputDispatcher;

            using WindowDimensionMap = std::map<ui8, std::vector<WindowDimensions>>;
            using FullscreenModeMap  = std::map<ui8, std::vector<FullscreenMode>>;
        public:
            Window() :
                m_window(nullptr),
                m_context(nullptr),
                m_initialised(false)
            { /* Empty. */ }
            ~Window() { /* Empty. */ }

            WindowError init(WindowSettings settings = {});
            void        dispose();

                   const char* name()                { return m_settings.name;                               }
                          ui32 window_id()           { return m_window_id;                                   }
              WindowDimensions dimensions()          { return m_settings.dimensions;                         }
                          ui32 width()               { return m_settings.dimensions.width;                   }
                          ui32 height()              { return m_settings.dimensions.height;                  }
                          ui32 display_idx()         { return m_settings.display_idx;                        }
                FullscreenMode fullscreen_mode()     { return m_settings.fullscreen_mode;                    }
                        ui32v2 aspect_ratio()        { return m_aspect_ratio;                                }
                           f32 aspect_ratio_frac()   { return (f32)m_aspect_ratio.x / (f32)m_aspect_ratio.y; }
                          bool fake_fullscreen()     { return m_settings.fake_fullscreen;                    }
                          bool is_fullscreen()       { return m_settings.is_fullscreen;                      }
                          bool is_resizable()        { return m_settings.is_resizable;                       }
                          bool is_borderless()       { return m_settings.is_borderless;                      }
                          bool is_maximised()        { return m_settings.is_maximised;                       }
                  SwapInterval swap_interval()       { return m_settings.swap_interval;                      }
            WindowDimensionMap allowed_resolutions() { return m_allowed_resolutions;                         }
             FullscreenModeMap fullscreen_modes()    { return m_fullscreen_modes;                            }

            void set_name(CALLEE_DELETE char* name);
            void set_dimensions(WindowDimensions dimensions);
            void set_width(ui32 width);
            void set_height(ui32 height);
            void set_display(ui32 display_idx);
            void set_fullscreen_mode(FullscreenMode fullscreen_mode);
            void set_fake_fullscreen(bool fake_fullscreen);
            void set_is_fullscreen(bool is_fullscreen);
            void set_is_resizable(bool is_resizable);
            void set_is_borderless(bool is_borderless);
            void set_is_maximised(bool is_maximised);
            void set_swap_interval(SwapInterval swapInterval);
            void set_allowed_resolutions(WindowDimensionMap allowed_resolutions);

            void check_display_occupied();

            void request_close();

            void sync();

            Event<>                          on_display_change;
            Event<>                          on_window_close;
            Event<>                          on_window_minimise;
            Event<>                          on_window_maximise;
            Event<>                          on_window_fullscreen_enter;
            Event<>                          on_window_fullscreen_exit;
            Event<ResizeEvent>               on_window_resize;
            Event<FullscreenModeChangeEvent> on_fullscreen_mode_change;
        private:
            void determine_modes();
            void calculate_aspect_ratio();
            void validate_dimensions();
            void validate_fullscreen_mode();

            bool               m_initialised;

            ui32 m_window_id;

            SDL_Window*        m_window;
            SDL_GLContext      m_context;

            WindowSettings     m_settings;
            ui32v2             m_aspect_ratio;
            WindowDimensionMap m_allowed_resolutions;
            FullscreenModeMap  m_fullscreen_modes;
        };
    }
}
namespace hg = hemlock::graphics;

#endif // __hemlock_graphics_window_h

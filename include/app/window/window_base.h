#ifndef __hemlock_app_window_window_base_h
#define __hemlock_app_window_window_base_h

#include "app/window/state.hpp"
#include "ui/input/events.hpp"

namespace hemlock {
    namespace app {
        class WindowBase {
        public:
            WindowBase();

            virtual ~WindowBase() { /* Empty. */
            }

            virtual WindowError init(WindowSettings settings = {}) = 0;
            virtual void        dispose()                          = 0;

            ui32 window_id() { return m_window_id; }

            const std::string& name() { return m_settings.name; }

            WindowDimensions dimensions() { return m_settings.dimensions; }

            ui32 width() { return m_settings.dimensions.width; }

            ui32 height() { return m_settings.dimensions.height; }

            ui32 display_idx() { return m_settings.display_idx; }

            FullscreenMode fullscreen_mode() { return m_settings.fullscreen_mode; }

            ui32v2 aspect_ratio() { return m_aspect_ratio; }

            f32 aspect_ratio_frac() {
                return static_cast<f32>(m_aspect_ratio.x)
                       / static_cast<f32>(m_aspect_ratio.y);
            }

            bool fake_fullscreen() { return m_settings.fake_fullscreen; }

            bool is_fullscreen() { return m_settings.is_fullscreen; }

            bool is_resizable() { return m_settings.is_resizable; }

            bool is_borderless() { return m_settings.is_borderless; }

            bool is_maximised() { return m_settings.is_maximised; }

            SwapInterval swap_interval() { return m_settings.swap_interval; }

            WindowDimensionMap allowed_resolutions() { return m_allowed_resolutions; }

            FullscreenModeMap fullscreen_modes() { return m_fullscreen_modes; }

            virtual void set_name(const std::string& name)             = 0;
            virtual void set_dimensions(WindowDimensions dimensions)   = 0;
            virtual void set_internal_dimensions(WindowDimensions dimensions) = 0;
            virtual void set_width(ui32 width)                         = 0;
            virtual void set_internal_width(ui32 width)                         = 0;
            virtual void set_height(ui32 height)                       = 0;
            virtual void set_internal_height(ui32 height)                       = 0;
            virtual void set_display(ui32 display_idx)                 = 0;
            virtual void set_fake_fullscreen(bool fake_fullscreen)     = 0;
            virtual void set_is_fullscreen(bool is_fullscreen)         = 0;
            virtual void set_is_resizable(bool is_resizable)           = 0;
            virtual void set_is_borderless(bool is_borderless)         = 0;
            virtual void set_is_maximised(bool is_maximised)           = 0;
            virtual void set_swap_interval(SwapInterval swap_interval) = 0;
            virtual void set_allowed_resolutions(WindowDimensionMap allowed_resolutions)
                = 0;
            virtual void set_fullscreen_mode(FullscreenMode fullscreen_mode) = 0;

            virtual void sync() = 0;

            Event<>                          on_display_change;
            Event<>                          on_window_close;
            Event<>                          on_window_minimise;
            Event<>                          on_window_maximise;
            Event<>                          on_window_fullscreen_enter;
            Event<>                          on_window_fullscreen_exit;
            Event<ResizeEvent>               on_window_resize;
            Event<FullscreenModeChangeEvent> on_fullscreen_mode_change;
        protected:
            Subscriber<hui::WindowMoveEvent>   handle_external_window_move;
            Subscriber<hui::WindowResizeEvent> handle_external_window_resize;

            virtual void check_display_occupied() = 0;

            void calculate_aspect_ratio();

            void validate_dimensions();
            void validate_fullscreen_mode();

            bool m_initialised;

            ui32               m_window_id;
            WindowSettings     m_settings;
            ui32v2             m_aspect_ratio;
            WindowDimensionMap m_allowed_resolutions;
            FullscreenModeMap  m_fullscreen_modes;
        };
    }  // namespace app
}  // namespace hemlock
namespace happ = hemlock::app;

#endif  // __hemlock_app_window_window_base_h

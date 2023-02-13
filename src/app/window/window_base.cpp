#include "stdafx.h"

#include "app/window/window_base.h"

happ::WindowBase::WindowBase() :
    handle_external_window_move(Subscriber<hui::WindowMoveEvent>{
        [&](Sender, hui::WindowMoveEvent ev) {
            if (ev.window_id != m_window_id) return;
            check_display_occupied();
        } }),
    handle_external_window_resize(Subscriber<hui::WindowResizeEvent>{
        [&](Sender, hui::WindowResizeEvent ev) {
            if (ev.window_id != m_window_id) return;
            set_internal_dimensions({
                {static_cast<ui32>(ev.width), static_cast<ui32>(ev.height)}
            });
        } }),
    m_initialised(false) { /* Empty. */
}

void happ::WindowBase::calculate_aspect_ratio() {
    Delegate<ui32(ui32, ui32)> calculateGCD
        = Delegate<ui32(ui32, ui32)>{ [&calculateGCD](ui32 x, ui32 y) {
              if (y > x) return calculateGCD(y, x);
              if (y == 0) return x;

              return calculateGCD(y, x % y);
          } };

    ui32 width = this->width(), height = this->height();
    ui32 gcd = calculateGCD(width, height);

    m_aspect_ratio = { width / gcd, height / gcd };
}

void happ::WindowBase::validate_dimensions() {
    // Track closest allowed dimension.
    WindowDimensions closest_dimensions{ m_settings.dimensions };
    ui32             distance2 = std::numeric_limits<ui32>::max();

    // Check each allowed resolution on the display currently occupied by the window.
    bool current_dimensions_allowed = false;
    for (auto& resolution : m_allowed_resolutions[m_settings.display_idx]) {
        if (m_settings.dimensions == resolution) {
            current_dimensions_allowed = true;
            break;
        }

        // Is this resolution "closer" to the current dimensions of the window?
        ui32 new_distance2 = static_cast<ui32>(
            std::pow(closest_dimensions.width - resolution.width, 2)
            + std::pow(closest_dimensions.height - resolution.height, 2)
        );
        if (new_distance2 < distance2) {
            closest_dimensions = resolution;
            distance2          = new_distance2;
        }
    }

    // If the current dimensions aren't allowed, change to
    // the dimensions we found to be the closest allowed.
    if (!current_dimensions_allowed) {
        set_dimensions(closest_dimensions);
    }
}

void happ::WindowBase::validate_fullscreen_mode() {
    // Check the current fullscreen mode is allowed on the new display.
    // If not change the mode to a reasonable alternative.
    auto old_setting_it = std::find_if(
        m_fullscreen_modes[m_settings.display_idx].begin(),
        m_fullscreen_modes[m_settings.display_idx].end(),
        [&](auto& it) { return it == m_settings.fullscreen_mode; }
    );
    if (old_setting_it == m_fullscreen_modes[m_settings.display_idx].end()) {
        /***********************************************************\
         * Test for a fullscreen mode with given resolution width. *
         * If found, set that as the new fullscreen mode.          *
        \***********************************************************/
        auto try_res_width = [&](ui32 width) -> bool {
            auto target_width_setting_it = std::find_if(
                m_fullscreen_modes[m_settings.display_idx].begin(),
                m_fullscreen_modes[m_settings.display_idx].end(),
                [width](const FullscreenMode& mode) {
                    return mode.resolution.width == width;
                }
            );
            if (target_width_setting_it
                != m_fullscreen_modes[m_settings.display_idx].end())
            {
                m_settings.fullscreen_mode = *target_width_setting_it;
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

bool operator==(const happ::WindowDimensions& lhs, const happ::WindowDimensions& rhs) {
    return lhs.width == rhs.width && lhs.height == rhs.height;
}

bool operator!=(const happ::WindowDimensions& lhs, const happ::WindowDimensions& rhs) {
    return !(lhs == rhs);
}

bool operator==(const happ::FullscreenMode& lhs, const happ::FullscreenMode& rhs) {
    return lhs.resolution == rhs.resolution && lhs.refresh_rate == rhs.refresh_rate
           && lhs.pixel_format == rhs.pixel_format;
}

bool operator!=(const happ::FullscreenMode& lhs, const happ::FullscreenMode& rhs) {
    return !(lhs == rhs);
}

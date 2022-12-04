#ifndef __hemlock_app_window_window_h
#define __hemlock_app_window_window_h

#include "app/window/window_base.h"
#include "ui/input/events.hpp"

namespace hemlock {
    namespace app {
#if defined(HEMLOCK_USING_VULKAN)
        struct H_VkExtensions {
            ui32         count;
            const char** names;
        };

        struct H_VkPhysicalDevices {
            ui32               count;
            VkPhysicalDevices* devices;
        };

        const VkApplicationInfo APP_INFO
            = { .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext              = nullptr,
                .pApplicationName   = APPLICATION_NAME,
                .applicationVersion = APPLICATION_VERSION,
                .pEngineName        = HEMLOCK_ENGINE_NAME,
                .engineVersion      = HEMLOCK_ENGINE_VERSION,
                .apiVersion         = VK_API_VERSION_1_3 };

        using DeviceEvaluator = Delegate<i32(VkPhysicalDevice)>;
#endif

        /**
         * @brief A sensible basic but full implementation of a window class.
         */
        class Window : public WindowBase {
        public:
            Window();

            virtual ~Window() { /* Empty. */
            }

            virtual WindowError init(WindowSettings settings = {}) override;
            virtual void        dispose() override;

            virtual void set_name(const std::string& name) override;
            virtual void set_dimensions(WindowDimensions dimensions) override;
            virtual void set_width(ui32 width) override;
            virtual void set_height(ui32 height) override;
            virtual void set_display(ui32 display_idx) override;
            virtual void set_fake_fullscreen(bool fake_fullscreen) override;
            virtual void set_is_fullscreen(bool is_fullscreen) override;
            virtual void set_is_resizable(bool is_resizable) override;
            virtual void set_is_borderless(bool is_borderless) override;
            virtual void set_is_maximised(bool is_maximised) override;
            virtual void set_swap_interval(SwapInterval swap_interval) override;
            virtual void
            set_allowed_resolutions(WindowDimensionMap allowed_resolutions) override;
            virtual void set_fullscreen_mode(FullscreenMode fullscreen_mode) override;

            virtual void sync() override;

#if defined(HEMLOCK_USING_VULKAN)
            /**
             * @brief Set the device evaluator used to
             * choose what hardware to render with.
             *
             * This must be called before init(...) to
             * take effect.
             *
             * @param evaluator The evaluator.
             */
            void set_device_evaluator(DeviceEvaluator&& evaluator);
#endif  // defined(HEMLOCK_USING_VULKAN)
        protected:
            virtual void check_display_occupied() override;
        private:
#if defined(HEMLOCK_USING_VULKAN)
            void determine_devices();
#endif  // defined(HEMLOCK_USING_VULKAN)
            void determine_modes();

#if defined(HEMLOCK_USING_SDL)
            SDL_Window* m_window;
#  if defined(HEMLOCK_USING_OPENGL)
            SDL_GLContext m_context;
#  endif  // defined(HEMLOCK_USING_OPENGL)
#endif    // defined(HEMLOCK_USING_SDL)
#if defined(HEMLOCK_USING_VULKAN)
            DeviceEvaluator     m_evaluator;
            VkInstance          m_instance;
            VkPhysicalDevice    m_device;
            VkSurfaceKHR*       m_surface;
            H_VkExtensions      m_extensions;
            H_VkPhysicalDevices m_available_devices;
#endif  // defined(HEMLOCK_USING_VULKAN)
        };
    }  // namespace app
}  // namespace hemlock
namespace happ = hemlock::app;

#endif  // __hemlock_app_window_window_h

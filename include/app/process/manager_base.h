#ifndef __hemlock_app_process_manager_base_h
#define __hemlock_app_process_manager_base_h

namespace hemlock {
    namespace app {
        class AppBase;

        class WindowBase;
        using Windows = std::unordered_map<ui32, WindowBase*>;

        class ProcessManagerBase {
        public:
            ProcessManagerBase() :
                m_main_window(nullptr),
                m_app(nullptr)
            { /* Empty. */ };
            virtual ~ProcessManagerBase() { /* Empty. */ }

            virtual WindowError init(AppBase* app) = 0;
            virtual void dispose() = 0;

            virtual void sync_windows() = 0;

            Window* main_window() { return m_main_window; }
        protected:
            Window* m_main_window;

            AppBase* m_app;
        };
    }
}
namespace hg = hemlock::graphics;

#endif // __hemlock_app_process_manager_base_h

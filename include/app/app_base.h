#ifndef __hemlock_app_app_base_h
#define __hemlock_app_app_base_h

namespace hemlock {
    class FpsLimiter;
    namespace ui {
        class InputManager;
    }

    namespace app {
        class ProcessManagerBase;
        class ScreenBase;

        struct ScreenChangeEvent {
            ScreenBase *before, *now;
        };

        class AppBase {
        protected:
            using ScreenList = std::unordered_map<std::string, ScreenBase*>;
            using Screen     = std::pair<std::string, ScreenBase*>;
        public:
            AppBase();
            virtual ~AppBase() { /* Empty */ };

            virtual void init()    = 0;
            virtual void dispose() = 0;

            virtual void run() = 0;

            bool should_quit() { return m_should_quit; }
            void set_should_quit(bool should_quit = true);

            bool change_screen(std::string name);

            ProcessManagerBase* process_manager() const { return m_process_manager; }

            Event<>                  on_quit;
        protected:
            Subscriber<> handle_external_quit;

            virtual void quit();

            bool m_initialised = false;

            bool m_should_quit = false;

            ProcessManagerBase* m_process_manager;
        };
    }
}
namespace happ = hemlock::app;

#endif // __hemlock_app_app_base_h

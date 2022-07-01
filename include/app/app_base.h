#ifndef __hemlock_app_app_base_h
#define __hemlock_app_app_base_h

namespace hemlock {
    class FpsLimiter;
    namespace ui {
        class InputManager;
    }

    namespace app {
        class ProcessManagerBase;

        class AppBase {
        public:
            AppBase();
            virtual ~AppBase() { /* Empty */ };

            virtual void run() = 0;

            bool should_quit() { return m_should_quit; }
            void set_should_quit(bool should_quit = true);

            ProcessManagerBase* process_manager() const { return m_process_manager; }

            Event<> on_quit;
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

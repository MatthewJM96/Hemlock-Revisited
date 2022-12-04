#ifndef __hemlock_app_process_manager_base_h
#define __hemlock_app_process_manager_base_h

namespace hemlock {
    namespace app {
        class AppBase;
        class ProcessBase;

        using Processes = std::unordered_map<ui32, ProcessBase*>;

        class ProcessManagerBase {
        public:
            ProcessManagerBase();

            virtual ~ProcessManagerBase() { /* Empty. */
            }

            virtual void init(AppBase* app);
            virtual void dispose();

            void set_quit_on_main_process_end(bool should = true) {
                m_quit_on_main_process_end = should;
            }

            ProcessBase* main_process() { return m_main_process; }

            ProcessBase* process(ui32 id);

            virtual void end_process(ui32 id);
        protected:
            /**
             * @brief Allows easy injection of extra logic in
             * situations such as multi-threaded processes.
             */
            virtual void end_processes() { /* Empty. */
            }

            bool m_initialised;

            bool m_quit_on_main_process_end;

            Processes    m_processes;
            ProcessBase* m_main_process;

            AppBase* m_app;
        };
    }  // namespace app
}  // namespace hemlock
namespace happ = hemlock::app;

#endif  // __hemlock_app_process_manager_base_h

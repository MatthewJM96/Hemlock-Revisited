#ifndef __hemlock_app_single_window_app_h
#define __hemlock_app_single_window_app_h

#include "app/app_base.h"
#include "app/process/process_base.h"
#include "ui/input/manager.h"

namespace hemlock {
    namespace app {
        class ScreenBase;

        class SingleWindowApp : public AppBase, public ProcessBase {
        public:
            SingleWindowApp() : AppBase(), ProcessBase(){ /* Empty */ };
            virtual ~SingleWindowApp(){ /* Empty */ };

            virtual void init() override;
            virtual void init(CALLEE_DELETE FrameTimer* timer);
            virtual void init(f32 target_fps, size_t tracked_frames_count = 5);
            virtual void dispose() override;

            virtual void run() override;

            hui::InputManager* input_manager() const {
                return const_cast<hui::InputManager*>(&m_input_manager);
            }
        protected:
            virtual void prepare_window() override;

            virtual void end_process() override { dispose(); }

            hui::InputManager m_input_manager;
        };
    }  // namespace app
}  // namespace hemlock
namespace happ = hemlock::app;

#endif  // __hemlock_app_single_window_app_h

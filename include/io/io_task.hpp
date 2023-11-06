#ifndef __hemlock_io_io_task_hpp
#define __hemlock_io_io_task_hpp

namespace hemlock {
    namespace io {
        class IOManagerBase;

        using IOTaskThreadState = thread::BasicThreadState;
        using IOTaskTaskQueue   = thread::TaskQueue<IOTaskThreadState>;

        class IOTask : public thread::IThreadTask<IOTaskThreadState> {
        public:
            void init(IOManagerBase* iomanager) { m_iomanager = iomanager; }
        protected:
            IOManagerBase* m_iomanager;
        };
    }  // namespace io
}  // namespace hemlock
namespace hio = hemlock::io;

#endif  // __hemlock_io_io_task_hpp

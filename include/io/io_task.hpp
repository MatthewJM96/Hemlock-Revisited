#ifndef __hemlock_io_io_task_hpp
#define __hemlock_io_io_task_hpp

namespace hemlock {
    namespace io {
        class IOManagerBase;

        using IOTaskThreadState = thread::Thread<thread::BasicThreadContext>::State;
        using IOTaskTaskQueue   = thread::TaskQueue<thread::BasicThreadContext>;

        class IOTask : public thread::IThreadTask<thread::BasicThreadContext> {
        public:
            void init(IOManagerBase* iomanager) { m_iomanager = iomanager; }
        protected:
            IOManagerBase* m_iomanager;
        };
    }  // namespace io
}  // namespace hemlock
namespace hio = hemlock::io;

#endif  // __hemlock_io_io_task_hpp

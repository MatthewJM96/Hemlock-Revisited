#ifndef __hemlock_io_io_task_hpp
#define __hemlock_io_io_task_hpp

namespace hemlock {
    namespace io {
        class IOManagerBase;

        class IOTask : public thread::IThreadTask {
        public:
            void init(IOManagerBase* iomanager) { m_iomanager = iomanager; }
        protected:
            IOManagerBase* m_iomanager;
        };
    }  // namespace io
}  // namespace hemlock
namespace hio = hemlock::io;

#endif  // __hemlock_io_io_task_hpp

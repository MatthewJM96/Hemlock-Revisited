#ifndef __hemlock_voxel_io_chunk_file_task_hpp
#define __hemlock_voxel_io_chunk_file_task_hpp

#include "io/io_task.hpp"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        using ChunkFileTaskThreadState = io::IOTaskThreadState;
        using ChunkFileTaskTaskQueue   = io::IOTaskTaskQueue;

        class ChunkFileTask : public io::IOTask {
        public:
            void init(hmem::WeakHandle<Chunk> chunk, io::IOManagerBase* iomanager) {
                io::IOTask::init(iomanager);

                m_chunk = chunk;
            }
        protected:
            hmem::WeakHandle<Chunk> m_chunk;
            static hio::fs::mapped_file chunk_data_file;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_io_chunk_file_task_hpp

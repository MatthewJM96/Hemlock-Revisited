#ifndef __hemlock_voxel_io_chunk_file_task_hpp
#define __hemlock_voxel_io_chunk_file_task_hpp

#include "io/io_task.hpp"
#include "voxel/chunk/decorator/decorator.hpp"

namespace hemlock {
    namespace voxel {
        template <ChunkDecorator... Decorations>
        struct Chunk;

        using ChunkFileTaskThreadState = io::IOTaskThreadState;
        using ChunkFileTaskTaskQueue   = io::IOTaskTaskQueue;

        template <ChunkDecorator... Decorations>
        class ChunkFileTask : public io::IOTask {
        public:
            void init(
                hmem::WeakHandle<Chunk<Decorations...>> chunk,
                io::IOManagerBase*                      iomanager
            ) {
                io::IOTask::init(iomanager);

                m_chunk = chunk;
            }
        protected:
            hmem::WeakHandle<Chunk<Decorations...>> m_chunk;
            // static hio::fs::mapped_file chunk_data_file;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_io_chunk_file_task_hpp

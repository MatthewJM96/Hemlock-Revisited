#ifndef __hemlock_voxel_chunk_loader_h
#define __hemlock_voxel_chunk_loader_h

#include "voxel/chunk.h"

namespace hemlock {
    namespace voxel {
        class ChunkLoadTask : public IThreadTask<ChunkGenTaskContext> {
        public:
            void init(Chunk* chunk);

            virtual void execute(ChunkGenThreadState* state, ChunkGenTaskQueue* task_queue) override;
        protected:
            Chunk* m_chunk;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_loader_h

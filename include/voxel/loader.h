#ifndef __hemlock_voxel_chunk_loader_h
#define __hemlock_voxel_chunk_loader_h

#include "voxel/chunk.h"

namespace hemlock {
    namespace voxel {
        class ChunkLoadTask : public ChunkGenTask {
        public:
            virtual void execute(ChunkGenThreadState* state, ChunkGenTaskQueue* task_queue) override;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_loader_h

#ifndef __hemlock_voxel_chunk_generator_h
#define __hemlock_voxel_chunk_generator_h

#include "voxel/chunk.h"

namespace hemlock {
    namespace voxel {
        class ChunkGenerationTask : public ChunkLoadTask {
        public:
            virtual void execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) override;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_generator_h

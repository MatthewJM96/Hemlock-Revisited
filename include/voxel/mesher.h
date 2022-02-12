#ifndef __hemlock_voxel_chunk_mesher_h
#define __hemlock_voxel_chunk_mesher_h

#include "voxel/chunk.hpp"

namespace hemlock {
    namespace voxel {
        class ChunkMeshTask : IThreadTask<ChunkGenTaskContext> {
        public:
            virtual void execute(ChunkGenThreadState* state, ChunkGenTaskQueue* task_queue) override;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_mesher_h

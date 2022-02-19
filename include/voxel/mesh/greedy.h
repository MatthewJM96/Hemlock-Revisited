#ifndef __hemlock_voxel_chunk_mesh_greedy_h
#define __hemlock_voxel_chunk_mesh_greedy_h

#include "voxel/chunk.h"

namespace hemlock {
    namespace voxel {
        class ChunkGreedyMeshTask : public ChunkLoadTask {
        public:
            virtual void execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) override;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_mesh_greedy_h

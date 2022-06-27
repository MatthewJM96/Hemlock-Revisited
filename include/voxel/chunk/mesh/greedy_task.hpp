#ifndef __hemlock_voxel_chunk_mesh_greedy_h
#define __hemlock_voxel_chunk_mesh_greedy_h

#include "voxel/chunk/task.hpp"
#include "voxel/chunk/mesh/mesh_task.hpp"

namespace hemlock {
    namespace voxel {
        template <hvox::ChunkMeshComparator MeshComparator>
        class ChunkGreedyMeshTask : public ChunkTask {
        public:
            virtual ~ChunkGreedyMeshTask() { /* Empty. */ }

            virtual void execute(ChunkLoadThreadState* state, ChunkTaskQueue* task_queue) override;
        };
    }
}
namespace hvox = hemlock::voxel;

#include "voxel/chunk/mesh/greedy_task.inl"

#endif // __hemlock_voxel_chunk_mesh_greedy_h

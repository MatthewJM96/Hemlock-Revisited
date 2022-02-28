#ifndef __hemlock_voxel_chunk_mesh_naive_h
#define __hemlock_voxel_chunk_mesh_naive_h

#include "voxel/chunk/load_task.hpp"
#include "voxel/chunk/mesh/mesh_task.hpp"

namespace hemlock {
    namespace voxel {
        template <ChunkMeshComparator MeshComparator>
        class ChunkNaiveMeshTask : public ChunkLoadTask {
        public:
            virtual void execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) override;
        };
    }
}
namespace hvox = hemlock::voxel;

#include "voxel/chunk/mesh/naive_task.inl"

#endif // __hemlock_voxel_chunk_mesh_naive_h

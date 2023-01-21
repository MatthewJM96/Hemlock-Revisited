#ifndef __hemlock_voxel_graphics_mesh_naive_h
#define __hemlock_voxel_graphics_mesh_naive_h

#include "voxel/predicate.hpp"
#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        template <hvox::IdealBlockComparator MeshComparator>
        class ChunkNaiveMeshTask : public ChunkTask {
        public:
            virtual void
            execute(ChunkLoadThreadState* state, ChunkTaskQueue* task_queue) override;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/graphics/mesh/naive_task.inl"

#endif  // __hemlock_voxel_graphics_mesh_naive_h

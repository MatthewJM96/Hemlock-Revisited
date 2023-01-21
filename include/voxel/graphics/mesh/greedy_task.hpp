#ifndef __hemlock_voxel_graphics_mesh_greedy_h
#define __hemlock_voxel_graphics_mesh_greedy_h

#include "voxel/predicate.hpp"
#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        template <hvox::IdealBlockComparator MeshComparator>
        class ChunkGreedyMeshTask : public ChunkTask {
        public:
            virtual ~ChunkGreedyMeshTask() { /* Empty. */
            }

            virtual void
            execute(ChunkLoadThreadState* state, ChunkTaskQueue* task_queue) override;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/graphics/mesh/greedy_task.inl"

#endif  // __hemlock_voxel_graphics_mesh_greedy_h

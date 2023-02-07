#ifndef __hemlock_voxel_ai_navmesh_task_hpp
#define __hemlock_voxel_ai_navmesh_task_hpp

#include "voxel/predicate.hpp"
#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        namespace ai {
            template <hvox::IdealBlockConstraint IsSolid>
            class ChunkNavmeshTask : public ChunkTask {
            public:
                virtual ~ChunkNavmeshTask() { /* Empty. */
                }

                virtual void execute(
                    ChunkLoadThreadState* state, ChunkTaskQueue* task_queue
                ) override;
            };
        }  // namespace ai
    }      // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "navmesh_task.inl"

#endif  // __hemlock_voxel_ai_navmesh_task_hpp

#ifndef __hemlock_voxel_ai_navmesh_navmesh_task_hpp
#define __hemlock_voxel_ai_navmesh_navmesh_task_hpp

#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        class ChunkGrid;

        namespace ai {
            /**
             * @brief Defines a struct whose opeartor() sets the blocks of a chunk.
             */
            template <typename StrategyCandidate>
            concept ChunkNavmeshStrategy = requires (
                StrategyCandidate s, hmem::Handle<ChunkGrid> g, hmem::Handle<Chunk> c
            ) {
                                               {
                                                   s.do_bulk(g, c)
                                                   } -> std::same_as<void>;
                                               {
                                                   s.do_stitch(g, c)
                                                   } -> std::same_as<void>;
                                           };

            template <hvox::ai::ChunkNavmeshStrategy NavmeshStrategy>
            class ChunkNavmeshTask : public ChunkTask {
            public:
                virtual ~ChunkNavmeshTask() { /* Empty. */
                }

                virtual void
                execute(ChunkThreadState* state, ChunkTaskQueue* task_queue) override;
            };
        }  // namespace ai
    }      // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "navmesh_task.inl"

#endif  // __hemlock_voxel_ai_navmesh_navmesh_task_hpp

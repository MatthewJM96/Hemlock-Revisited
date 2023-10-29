#ifndef __hemlock_voxel_graphics_mesh_mesh_task_hpp
#define __hemlock_voxel_graphics_mesh_mesh_task_hpp

#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        class ChunkGrid;
        struct Chunk;

        /**
         * @brief Defines a struct whose opeartor() sets the blocks of a chunk.
         */
        template <typename StrategyCandidate>
        concept ChunkMeshStrategy = requires (
            StrategyCandidate s, hmem::Handle<ChunkGrid> g, entt::entity c
        ) {
                                        {
                                            s.can_run(g, c)
                                            } -> std::same_as<bool>;
                                        {
                                            s.operator()(g, c)
                                            } -> std::same_as<void>;
                                    };

        template <hvox::ChunkMeshStrategy MeshStrategy>
        class ChunkMeshTask : public ChunkTask {
        public:
            virtual ~ChunkMeshTask() { /* Empty. */
            }

            virtual void
            execute(ChunkLoadThreadState* state, ChunkTaskQueue* task_queue) override;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/graphics/mesh/mesh_task.inl"

#endif  // __hemlock_voxel_graphics_mesh_mesh_task_hpp

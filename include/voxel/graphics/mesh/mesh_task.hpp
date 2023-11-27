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
            StrategyCandidate s, hmem::Handle<ChunkGrid> g, hmem::Handle<Chunk> c
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

            virtual bool execute(hthread::QueueDelegate* queue_task) override;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/graphics/mesh/mesh_task.inl"

#endif  // __hemlock_voxel_graphics_mesh_mesh_task_hpp

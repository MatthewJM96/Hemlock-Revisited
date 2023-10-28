#ifndef __hemlock_voxel_cellular_automata_ca_task_hpp
#define __hemlock_voxel_cellular_automata_ca_task_hpp

#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        /**
         * @brief Defines a struct whose opeartor() sets the blocks of a chunk.
         */
        template <typename StrategyCandidate>
        concept CelluarAutomataStrategy = requires (
            StrategyCandidate s, hmem::Handle<ChunkGrid> g, hmem::Handle<Chunk> c
        ) {
                                              {
                                                  s.can_run(g, c)
                                                  } -> std::same_as<bool>;
                                              {
                                                  s.operator()(g, c)
                                                  } -> std::same_as<void>;
                                          };

        template <hvox::CelluarAutomataStrategy CaStrategy>
        class CellularAutomataTask : public ChunkTask {
        public:
            virtual ~CellularAutomataTask() { /* Empty. */
            }

            virtual void execute(
                ChunkTaskThreadState* state, ChunkTaskQueue* task_queue
            ) override final;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/cellular_automata/ca_task.inl"

#endif  // __hemlock_voxel_cellular_automata_ca_task_hpp

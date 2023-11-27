#ifndef __hemlock_voxel_generation_generator_task_hpp
#define __hemlock_voxel_generation_generator_task_hpp

#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        /**
         * @brief Defines a struct whose opeartor() sets the blocks of a chunk.
         */
        template <typename StrategyCandidate>
        concept ChunkGenerationStrategy
            = requires (StrategyCandidate s, hmem::Handle<Chunk> c) {
                  {
                      s.operator()(c)
                      } -> std::same_as<void>;
              };

        template <hvox::ChunkGenerationStrategy GenerationStrategy>
        class ChunkGenerationTask : public ChunkTask {
        public:
            virtual ~ChunkGenerationTask() { /* Empty. */
            }

            virtual bool
            execute(ChunkThreadState* state, hthread::QueueDelegate* queue_task) override;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/generation/generator_task.inl"

#endif  // __hemlock_voxel_generation_generator_task_hpp

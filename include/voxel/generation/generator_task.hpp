#ifndef __hemlock_voxel_generation_generator_task_hpp
#define __hemlock_voxel_generation_generator_task_hpp

#include "voxel/chunk/decorator/decorator.hpp"
#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        template <ChunkDecorator... Decorations>
        struct Chunk;

        /**
         * @brief Defines a struct whose opeartor() sets the blocks of a chunk.
         */
        template <typename StrategyCandidate, typename... Decorations>
        concept ChunkGenerationStrategy
            = (ChunkDecorator<Decorations> && ...)
              && requires (StrategyCandidate s, hmem::Handle<Chunk<Decorations...>> c) {
                     {
                         s.operator()(c)
                         } -> std::same_as<void>;
                 };

        template <
            hvox::ChunkGenerationStrategy GenerationStrategy,
            ChunkDecorator... Decorations>
        class ChunkGenerationTask : public ChunkTask<Decorations...> {
        public:
            virtual ~ChunkGenerationTask() { /* Empty. */
            }

            virtual void
            execute(ChunkThreadState* state, ChunkTaskQueue* task_queue) override;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/generation/generator_task.inl"

#endif  // __hemlock_voxel_generation_generator_task_hpp

#ifndef __hemlock_voxel_chunk_generator_h
#define __hemlock_voxel_chunk_generator_h

#include "voxel/chunk/load_task.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        /**
         * @brief Defines a struct whose opeartor() sets the blocks of a chunk.
         */
        template <typename StrategyCandidate>
        concept ChunkGenerationStrategy = requires (
             StrategyCandidate s,
                        Chunk* c
        ) {
            { s.operator()(c) } -> std::same_as<void>;
        };

        template <ChunkGenerationStrategy GenerationStrategy>
        class ChunkGenerationTask : public ChunkLoadTask {
        public:
            virtual void execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) override;
        };
    }
}
namespace hvox = hemlock::voxel;

#include "voxel/chunk/generator_task.inl"

#endif // __hemlock_voxel_chunk_generator_h

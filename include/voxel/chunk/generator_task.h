#ifndef __hemlock_voxel_chunk_generator_h
#define __hemlock_voxel_chunk_generator_h

#include "voxel/chunk/load_task.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Block;

        using ChunkGenerationStrategy = Delegate<void(Block*, ChunkGridPosition)>;

        class ChunkGenerationTask : public ChunkLoadTask {
        public:
            virtual void execute(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) override;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_generator_h

#ifndef __hemlock_voxel_chunk_mesher_h
#define __hemlock_voxel_chunk_mesher_h

namespace hemlock {
    namespace voxel {
        struct Chunk;

        struct ChunkMeshTaskContext {
            volatile bool stop;
            Chunk*        chunk;
        };
        using ChunkMeshThreadState = Thread<ChunkMeshTaskContext>::State;
        using ChunkMeshTaskQueue   = TaskQueue<ChunkMeshTaskContext>;

        class ChunkMeshTask : IThreadTask<ChunkMeshTaskContext> {
        public:
            virtual void execute(ChunkMeshThreadState* state, ChunkMeshTaskQueue* task_queue) override;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_mesher_h

#ifndef __hemlock_voxel_chunk_load_task_hpp
#define __hemlock_voxel_chunk_load_task_hpp

namespace hemlock {
    namespace voxel {
        struct Chunk;
        class ChunkGrid;

        enum class ChunkLoadTaskKind : ui8 {
            NONE            = 0,
            GENERATION      = 1,
            MESH            = 2,
            MESH_UPLOAD     = 3
        };

        struct ChunkLoadTaskContext {
            volatile bool stop;
            volatile bool suspend;
        };
        using ChunkLoadThreadState = Thread<ChunkLoadTaskContext>::State;
        using ChunkLoadTaskQueue   = TaskQueue<ChunkLoadTaskContext>;
        class ChunkLoadTask : public IThreadTask<ChunkLoadTaskContext> {
        public:
            void init(Chunk* chunk, ChunkGrid* chunk_grid, void* strategy);
        protected:
            Chunk*     m_chunk;
            ChunkGrid* m_chunk_grid;
            void*      m_strategy;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_load_task_hpp

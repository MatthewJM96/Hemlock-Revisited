#ifndef __hemlock_voxel_chunk_load_task_hpp
#define __hemlock_voxel_chunk_load_task_hpp

namespace hemlock {
    namespace voxel {
        struct Chunk;
        class ChunkGrid;

        enum class ChunkTaskKind : ui8 {
            NONE = 0,
            GENERATION,
            MESH,
            MESH_UPLOAD,
        };

        using ChunkTaskContext     = thread::BasicThreadContext;
        using ChunkLoadThreadState = thread::Thread<ChunkTaskContext>::State;
        using ChunkTaskQueue       = thread::TaskQueue<ChunkTaskContext>;

        class ChunkTask : public thread::IThreadTask<ChunkTaskContext> {
        public:
            virtual ~ChunkTask() { /* Empty. */
            }

            void set_state(
                hmem::WeakHandle<Chunk> chunk, hmem::WeakHandle<ChunkGrid> chunk_grid
            );
        protected:
            hmem::WeakHandle<Chunk>     m_chunk;
            hmem::WeakHandle<ChunkGrid> m_chunk_grid;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_load_task_hpp

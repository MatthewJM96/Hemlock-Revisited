#ifndef __hemlock_voxel_task_hpp
#define __hemlock_voxel_task_hpp

#include "voxel/chunk/decorator/decorator.hpp"

namespace hemlock {
    namespace voxel {
        template <ChunkDecorator... Decorations>
        struct Chunk;

        template <ChunkDecorator... Decorations>
        class ChunkGrid;

        enum class ChunkTaskKind : ui8 {
            NONE = 0,
            GENERATION,
            MESH,
            MESH_UPLOAD,
        };

        using ChunkTaskContext = thread::BasicThreadContext;
        using ChunkThreadState = thread::Thread<ChunkTaskContext>::State;
        using ChunkTaskQueue   = thread::TaskQueue<ChunkTaskContext>;

        template <ChunkDecorator... Decorations>
        class ChunkTask : public thread::IThreadTask<ChunkTaskContext> {
        public:
            virtual ~ChunkTask() { /* Empty. */
            }

            void set_state(
                hmem::WeakHandle<Chunk<Decorations...>>     chunk,
                hmem::WeakHandle<ChunkGrid<Decorations...>> chunk_grid
            ) {
                m_chunk      = chunk;
                m_chunk_grid = chunk_grid;
            }
        protected:
            hmem::WeakHandle<Chunk<Decorations...>>     m_chunk;
            hmem::WeakHandle<ChunkGrid<Decorations...>> m_chunk_grid;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_task_hpp

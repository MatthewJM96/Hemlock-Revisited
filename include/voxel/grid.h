#ifndef __hemlock_voxel_grid_h
#define __hemlock_voxel_grid_h

#include "timing.h"
#include "voxel/chunk.h"

namespace hemlock {
    namespace voxel {
        // TODO(Matthew): We can do better storage of chunks and blocks at this level.
        // TODO(Matthew): We should add support for LOD, different generation stages, disabling meshing etc.

        using Chunks = std::unordered_map<ChunkID, Chunk*>;

        class ChunkGrid {
        public:
            ChunkGrid()  { /* Empty. */ }
            ~ChunkGrid() { /* Empty. */ }

            void init(ui32 thread_count);
            void dispose();

            void update(TimeData time);

            void suspend_chunk_tasks() { m_gen_threads.suspend(); }
            void resume_chunk_tasks()  { m_gen_threads.resume();  }

            bool load_chunk_at(ChunkGridPosition chunk_position);
            bool unload_chunk_at(ChunkGridPosition chunk_position);

            const Chunks& chunks() { return m_chunks; }
        protected:
            void establish_chunk_neighbours(Chunk* chunk);

            ThreadPool<ChunkGenTaskContext> m_gen_threads;

            Chunks m_chunks;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_grid_h

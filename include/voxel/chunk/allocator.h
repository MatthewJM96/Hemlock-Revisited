#ifndef __hemlock_voxel_chunk_allocator_h
#define __hemlock_voxel_chunk_allocator_h

#include "voxel/chunk/handle.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;
        struct ChunkHandle;

        using ChunkHandles = std::unordered_map<ChunkID, ChunkHandle>;

        class ChunkAllocator {
        public:
            ChunkAllocator()  { /* Empty. */ }
            ~ChunkAllocator() { /* Empty. */ }

            void dispose();

            ChunkHandle acquire(ChunkGridPosition pos);
            ChunkHandle acquire(ChunkID id);

            bool release(ChunkHandle& handle);
        protected:
            ChunkHandle allocate(ChunkID id);

            bool try_deallocate(ChunkHandle& handle);

            std::mutex   m_handles_mutex;
            ChunkHandles m_handles;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_allocator_h

#ifndef __hemlock_voxel_chunk_allocator_h
#define __hemlock_voxel_chunk_allocator_h

#include "voxel/chunk/handle.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;
        struct ChunkHandle;

        class ChunkAllocator {
        public:
            ChunkAllocator()  { /* Empty. */ }
            ~ChunkAllocator() { /* Empty. */ }

            void dispose();

            ChunkHandle& acquire(ChunkGridPosition pos);
            ChunkHandle& acquire(ChunkID id);

            bool release(ChunkHandle&& handle);
        protected:
            ChunkHandle& acquire_existing(ChunkID id);

            ChunkHandleMetadatas m_metadata;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_allocator_h

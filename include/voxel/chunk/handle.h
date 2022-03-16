#ifndef __hemlock_voxel_chunk_handle_h
#define __hemlock_voxel_chunk_handle_h

#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;
        class ChunkAllocator;

        class ChunkHandle {
            friend ChunkAllocator;
        public:
            ChunkHandle();
            ~ChunkHandle() { /* Empty. */ }

            ChunkHandle(const ChunkHandle&);
            ChunkHandle& operator=(const ChunkHandle&);

            ChunkHandle(ChunkHandle&&);
            ChunkHandle& operator=(ChunkHandle&&);

            Chunk& operator*();
            const Chunk& operator*() const;

            Chunk* operator->();
            const Chunk* operator->() const;

            bool operator==(void* possible_nullptr);
            bool operator==(const ChunkHandle& handle);

            bool operator!=(void* possible_nullptr);
            bool operator!=(const ChunkHandle& handle);

            bool release();
        protected:
            static ChunkHandle acquire_existing(const ChunkHandle& handle);

            ChunkHandle(Chunk* chunk);

            Chunk* m_chunk;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_handle_h

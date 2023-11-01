#ifndef __hemlock_voxel_chunk_state_hpp
#define __hemlock_voxel_chunk_state_hpp

namespace hemlock {
    namespace voxel {
        struct Chunk;

        enum class ChunkState : ui8 {
            NONE     = 0,
            PENDING  = 1,
            ACTIVE   = 2,
            COMPLETE = 3
        };

        enum class ChunkAliveState : ui8 {
            ALIVE,
            DEAD
        };

        using LODLevel = ui8;

        struct LODChangeEvent {
            LODLevel before, after;
        };

        union Neighbours {
            Neighbours() : all{} { /* Empty. */
            }

            Neighbours(const Neighbours& rhs) {
                for (size_t i = 0; i < 8; ++i) all[i] = rhs.all[i];
            }

            ~Neighbours() { /* Empty. */
            }

            Neighbours& operator=(const Neighbours& rhs) {
                for (size_t i = 0; i < 8; ++i) all[i] = rhs.all[i];
                return *this;
            }

            struct {
                hmem::WeakHandle<Chunk> left, right, top, bottom, front, back;
            } one;

            hmem::WeakHandle<Chunk> all[8];
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_state_hpp

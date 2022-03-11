#ifndef __hemlock_voxel_chunk_state_hpp
#define __hemlock_voxel_chunk_state_hpp

namespace hemlock {
    namespace voxel {
        struct Chunk;

        enum class ChunkState : ui8 {
            NONE            = 0,
            PRELOADED       = 1,
            GENERATED       = 2,
            MESHED          = 3,
            MESH_UPLOADED   = 4
        };

        enum class ChunkAliveState : ui8 {
            ALIVE,
            DEAD
        };

        enum class RenderState : ui8 {
            NONE    = 0,
            LOD_0   = 1,
            LOD_1   = 2,
            LOD_2   = 3,
            LOD_3   = 4,
            LOD_4   = 5,
            LOD_5   = 6,
            LOD_6   = 7,
            LOD_7   = 8,
            FULL    = 9
        };

        union Neighbours {
            Neighbours() :
                all({})
            { /* Empty. */ }
            Neighbours(const Neighbours& rhs) {
                for (size_t i = 0; i < 8; ++i) all[i] = rhs.all[i];
            }
            ~Neighbours() { /* Empty. */ }


            Neighbours& operator=(const Neighbours& rhs) {
                for (size_t i = 0; i < 8; ++i) all[i] = rhs.all[i];
                return *this;
            }

            struct {
                hmem::WeakHandle<Chunk> left, right, top, bottom, front, back;
            } one;
            hmem::WeakHandle<Chunk> all[8];
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_state_hpp

#ifndef __hemlock_voxel_block_hpp
#define __hemlock_voxel_block_hpp

namespace hemlock {
    namespace voxel {
        struct Block {
            ui64 id;
            // more stuff
        };
        const Block NULL_BLOCK = Block{0};
    }
}
namespace hvox = hemlock::voxel;

inline bool operator==(hvox::Block lhs, hvox::Block rhs) {
    return lhs.id == rhs.id;
}
inline bool operator!=(hvox::Block lhs, hvox::Block rhs) {
    return !(lhs == rhs);
}

#endif // __hemlock_voxel_block_hpp

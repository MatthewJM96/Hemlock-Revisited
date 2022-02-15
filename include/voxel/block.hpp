#ifndef __hemlock_voxel_block_hpp
#define __hemlock_voxel_block_hpp

#include "graphics/mesh.h"

namespace hemlock {
    namespace voxel {
        using BlockID = ui64;
        struct Block {
            BlockID id;
            // more stuff
        };
        const Block NULL_BLOCK = Block{0};

        const ui32 BLOCK_VERTEX_COUNT = 36;

        static hg::Colourless_Vertex3D_32* const BLOCK_VERTICES = new hg::Colourless_Vertex3D_32[BLOCK_VERTEX_COUNT] {
            { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
            { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
            { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
            { { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
            { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
            { { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } }
        };

        const hg::Colourless_MeshData3D_32 BLOCK_MESH = { BLOCK_VERTICES, BLOCK_VERTEX_COUNT };
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

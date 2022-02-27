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

        using BlockVertex   = hg::Colourless_3D_32_Normal_Vertex;
        using BlockMeshData = hg::Colourless_3D_32_Normal_MeshData;

        static BlockVertex* const BLOCK_VERTICES = new BlockVertex[BLOCK_VERTEX_COUNT] {
            /* FRONT */
            { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f }, {  0.0f,  0.0f, -1.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f }, {  0.0f,  0.0f, -1.0f } },
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f }, {  0.0f,  0.0f, -1.0f } },
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f }, {  0.0f,  0.0f, -1.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f }, {  0.0f,  0.0f, -1.0f } },
            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f }, {  0.0f,  0.0f, -1.0f } },
            /* BACK */
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f }, {  0.0f,  0.0f,  1.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f }, {  0.0f,  0.0f,  1.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f } },
            { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f }, {  0.0f,  0.0f,  1.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f }, {  0.0f,  0.0f,  1.0f } },
            /* LEFT */
            { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, { -1.0f,  0.0f,  0.0f } },
            { { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f }, { -1.0f,  0.0f,  0.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, { -1.0f,  0.0f,  0.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, { -1.0f,  0.0f,  0.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f }, { -1.0f,  0.0f,  0.0f } },
            { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, { -1.0f,  0.0f,  0.0f } },
            /* RIGHT */
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, {  1.0f,  0.0f,  0.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, {  1.0f,  0.0f,  0.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, {  1.0f,  0.0f,  0.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, {  1.0f,  0.0f,  0.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f }, {  1.0f,  0.0f,  0.0f } },
            /* BOTTOM */
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, {  0.0f, -1.0f,  0.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f }, {  0.0f, -1.0f,  0.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f }, {  0.0f, -1.0f,  0.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f }, {  0.0f, -1.0f,  0.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f }, {  0.0f, -1.0f,  0.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, {  0.0f, -1.0f,  0.0f } },
            /* TOP */
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f }, {  0.0f,  1.0f,  1.0f } },
            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f }, {  0.0f,  1.0f,  1.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, {  0.0f,  1.0f,  1.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, {  0.0f,  1.0f,  1.0f } },
            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f }, {  0.0f,  1.0f,  1.0f } },
            { { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f }, {  0.0f,  1.0f,  1.0f } }
        };

        const BlockMeshData BLOCK_MESH = { BLOCK_VERTICES, BLOCK_VERTEX_COUNT };
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

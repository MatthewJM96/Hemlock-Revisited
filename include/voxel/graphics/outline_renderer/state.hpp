#ifndef __hemlock_voxel_graphics_outline_renderer_state_hpp
#define __hemlock_voxel_graphics_outline_renderer_state_hpp

#include "graphics/mesh.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        template <typename Type>
        concept OutlinePredicate
            = RPredicate<std::tuple<bool, colour4>, Type, hmem::Handle<Chunk>>;

        struct OutlineData {
            f32v3   position;
            colour4 colour;
        };

        using OutlineVertex   = hg::Point_3D_32_Vertex;
        using OutlineMeshData = hg::ConstPoint_3D_32_MeshData;
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_graphics_outline_renderer_state_hpp

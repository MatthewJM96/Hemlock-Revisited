#ifndef __hemlock_voxel_ray_h
#define __hemlock_voxel_ray_h

namespace hemlock {
    namespace voxel {
        class ChunkGrid;

        class Ray {
        public:
            Ray();
            ~Ray();

            void init(hmem::WeakHandle<ChunkGrid> grid);
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_ray_h

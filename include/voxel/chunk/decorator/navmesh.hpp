#ifndef __hemlock_voxel_chunk_decorator_navmesh_hpp
#define __hemlock_voxel_chunk_decorator_navmesh_hpp

#include "thread/resource_guard.hpp"
#include "voxel/ai/navmesh.hpp"
#include "voxel/chunk/state.hpp"

namespace hemlock {
    namespace voxel {
        /**
         * @brief
         */
        struct NavmeshDecorator {
            hthread::ResourceGuard<ai::ChunkNavmesh> navmesh;

            std::atomic<ChunkState> bulk_navmeshing, navmeshing;

            struct {
                std::atomic<ChunkState> right, top, front, above_left, above_right,
                    above_front, above_back, above_and_across_left,
                    above_and_across_right, above_and_across_front,
                    above_and_across_back;
            } navmesh_stitch;

            Event<> on_navmesh_change;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_decorator_navmesh_hpp

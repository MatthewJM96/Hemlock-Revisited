#ifndef __hemlock_voxel_chunk_component_navmesh_hpp
#define __hemlock_voxel_chunk_component_navmesh_hpp

#include "voxel/ai/navmesh/navmesh_manager.h"
#include "voxel/chunk/state.hpp"

namespace hemlock {
    namespace voxel {
        /**
         * @brief
         */
        struct ChunkNavmeshComponent {
            // TODO(Matthew): navmesh wants to probably be paged in some amount of bulk
            //                and divied out, that or we need to stack allocate.
            ai::ChunkNavmeshManager navmesh;

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

#endif  // __hemlock_voxel_chunk_component_navmesh_hpp

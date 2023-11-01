#ifndef __hemlock_voxel_chunk_decorator_lod_hpp
#define __hemlock_voxel_chunk_decorator_lod_hpp

#include "voxel/chunk/state.hpp"

namespace hemlock {
    namespace voxel {
        /**
         * @brief
         */
        struct LODDecorator {
            std::atomic<LODLevel> lod_level;

            Event<LODChangeEvent> on_lod_change;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_decorator_lod_hpp

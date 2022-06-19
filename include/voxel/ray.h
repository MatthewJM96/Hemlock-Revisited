#ifndef __hemlock_voxel_ray_h
#define __hemlock_voxel_ray_h

#include "block.hpp"
#include "coordinate_system.h"

namespace hemlock {
    namespace voxel {
        class ChunkGrid;

        namespace Ray {
            using BlockTest = Delegate<bool(const Block&)>;

            /**
             * @brief Steps the ray along its direction until the
             * first block that is exactly the specified target block.
             *
             * @param start The location the ray starts at.
             * @param direction The direction to cast the ray in.
             * @param grid_handle Handle to the chunk grid in which the ray is cast.
             * @param target_block The target block that the ray will stop upon
             * reaching.
             * @param max_steps The maximum number of steps the ray should take
             * before failing.
             * @param position The position of the block the ray met, unchanged
             * if the ray was not incident on the target block.
             * @param distance The distance to the block the ray met, unchanged
             * if the ray was not incident on the target block.
             * @return True if the ray was incident on a block matching the
             * target within max_steps, false otherwise.
             */
            bool cast_to_block(           f32v3 start, 
                                          f32v3 direction,
                    hmem::WeakHandle<ChunkGrid> grid_handle,
                                          Block target_block,
                                           ui32 max_steps,
                        OUT BlockWorldPosition& position,
                                       OUT f32& distance    );


            /**
             * @brief Steps the ray along its direction until the
             * first block that satisfies the conditions encoded by
             * block_is_target.
             *
             * @param start The location the ray starts at.
             * @param direction The direction to cast the ray in.
             * @param grid_handle Handle to the chunk grid in which the ray is cast.
             * @param block_is_target The test function, must return false
             * except for a block considered a target of the ray.
             * @param max_steps The maximum number of steps the ray should take
             * before failing.
             * @param position The position of the block the ray met, unchanged
             * if the ray was not incident on the target block.
             * @param distance The distance to the block the ray met, unchanged
             * if the ray was not incident on the target block.
             * @return True if the ray was incident on a block matching the
             * target within max_steps, false otherwise.
             */
            bool cast_to_block(           f32v3 start,
                                          f32v3 direction,
                    hmem::WeakHandle<ChunkGrid> grid_handle,
                                      BlockTest block_is_target,
                                           ui32 max_steps,
                        OUT BlockWorldPosition& position,
                                       OUT f32& distance    );
        }
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_ray_h

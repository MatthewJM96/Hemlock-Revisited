#ifndef __hemlock_voxel_ray_h
#define __hemlock_voxel_ray_h

#include "voxel/coordinate_system.h"
#include "voxel/state.h"

namespace hemlock {
    namespace voxel {
        class ChunkGrid;

        namespace Ray {
            using VoxelTest = Delegate<bool(const Voxel&)>;

            /**
             * @brief Steps the ray along its direction until the
             * first voxel that is exactly the specified target voxel.
             *
             * @param start The location the ray starts at.
             * @param direction The normalised direction to cast the ray in.
             * @param grid_handle Handle to the chunk grid in which the ray is cast.
             * @param target_voxel The target voxel that the ray will stop upon
             * reaching.
             * @param max_steps The maximum number of steps the ray should take
             * before failing.
             * @param position The position of the voxel the ray met.
             * @param distance The distance to the voxel the ray met.
             * @param chunk The chunk inside which the ray met a target voxel.
             * @return True if the ray was incident on a voxel matching the
             * target within max_steps, false otherwise.
             */
            bool cast_to_voxel(
                f32v3                       start,
                f32v3                       direction,
                hmem::WeakHandle<ChunkGrid> grid_handle,
                Voxel                       target_voxel,
                ui32                        max_steps,
                OUT VoxelWorldPosition&     position,
                OUT f32&                    distance,
                OUT hmem::WeakHandle<Chunk>* chunk = nullptr
            );

            /**
             * @brief Steps the ray along its direction until the
             * first voxel that satisfies the conditions encoded by
             * voxel_is_target.
             *
             * @param start The location the ray starts at.
             * @param direction The normalised direction to cast the ray in.
             * @param grid_handle Handle to the chunk grid in which the ray is cast.
             * @param voxel_is_target The test function, must return false
             * except for a voxel considered a target of the ray.
             * @param max_steps The maximum number of steps the ray should take
             * before failing.
             * @param position The position of the voxel the ray met.
             * @param distance The distance to the voxel the ray met.
             * @param chunk The chunk inside which the ray met a target voxel.
             * @return True if the ray was incident on a voxel matching the
             * target within max_steps, false otherwise.
             */
            bool cast_to_voxel(
                f32v3                       start,
                f32v3                       direction,
                hmem::WeakHandle<ChunkGrid> grid_handle,
                VoxelTest                   voxel_is_target,
                ui32                        max_steps,
                OUT VoxelWorldPosition&     position,
                OUT f32&                    distance,
                OUT hmem::WeakHandle<Chunk>* chunk = nullptr
            );

            /**
             * @brief Steps the ray along its direction until the
             * first voxel that is exactly the specified target voxel.
             * The position and distance are set to the voxel the ray
             * was incident on before that voxel.
             *
             * @param start The location the ray starts at.
             * @param direction The normalised direction to cast the ray in.
             * @param grid_handle Handle to the chunk grid in which the ray is cast.
             * @param target_voxel The target voxel that the ray will stop upon
             * reaching.
             * @param max_steps The maximum number of steps the ray should take
             * before failing.
             * @param position The position of the voxel the ray met.
             * @param distance The distance to the voxel the ray met.
             * @param chunk The chunk inside which the ray met a target voxel.
             * @return True if the ray was incident on a voxel matching the
             * target within max_steps, false otherwise.
             */
            bool cast_to_voxel_before(
                f32v3                       start,
                f32v3                       direction,
                hmem::WeakHandle<ChunkGrid> grid_handle,
                Voxel                       target_voxel,
                ui32                        max_steps,
                OUT VoxelWorldPosition&     position,
                OUT f32&                    distance,
                OUT hmem::WeakHandle<Chunk>* chunk = nullptr
            );

            /**
             * @brief Steps the ray along its direction until the
             * first voxel that satisfies the conditions encoded by
             * voxel_is_target. The position and distance are set to
             * the voxel the ray was incident on before that voxel.
             *
             * @param start The location the ray starts at.
             * @param direction The normalised direction to cast the ray in.
             * @param grid_handle Handle to the chunk grid in which the ray is cast.
             * @param voxel_is_target The test function, must return false
             * except for a voxel considered a target of the ray.
             * @param max_steps The maximum number of steps the ray should take
             * before failing.
             * @param position The position of the voxel the ray met.
             * @param distance The distance to the voxel the ray met.
             * @param chunk The chunk inside which the ray met a target voxel.
             * @return True if the ray was incident on a voxel matching the
             * target within max_steps, false otherwise.
             */
            bool cast_to_voxel_before(
                f32v3                       start,
                f32v3                       direction,
                hmem::WeakHandle<ChunkGrid> grid_handle,
                VoxelTest                   voxel_is_target,
                ui32                        max_steps,
                OUT VoxelWorldPosition&     position,
                OUT f32&                    distance,
                OUT hmem::WeakHandle<Chunk>* chunk = nullptr
            );
        }  // namespace Ray
    }      // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_ray_h

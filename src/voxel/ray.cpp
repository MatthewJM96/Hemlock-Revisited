#include "stdafx.h"

#include "voxel/chunk/grid.h"
#include "voxel/ray.h"

// TODO(Matthew): so many ways to make this faster it hurts, but fine for now.

static i32 sign(f32 num) {
    if (num > 0.0f) return 1;
    if (num < 0.0f) return -1;
    return 0;
}

/**
 * @brief Returns smallest integer, t, that satisfies:
 *              s + t * ds = integer
 *
 * @param s Starting float.
 * @param ds Small delta float.
 * @return i32 Integer coefficient required to multiply ds
 * such that adding the result to s returns an integer.
 */
static f32 min_coeff_to_int(f32 s, f32 ds) {
    if (ds < 0.0f) return min_coeff_to_int(-s, -ds);

    // E.g. where ' refers to sign handling above,
    // and '' refers to fmod below.
    //
    //  |   s  |  ds  |  s'  |  ds'  |  s''  |  s/ds  |
    //  |------|------|------|-------|-------|--------|
    //  |  0.4 |  0.1 |  0.4 |  0.1  |  0.6  |    6   |
    //  |  0.4 | -0.1 | -0.4 |  0.1  |  0.4  |    4   |
    //  | -0.4 |  0.1 | -0.4 |  0.1  |  0.4  |    4   |
    //  | -0.4 | -0.1 |  0.4 |  0.1  |  0.6  |    6   |

    s = abs(fmod(fmod(s, 1.0f) - 1.0f, 1.0f));

    return s / ds;
}

/**
 * @brief Steps a ray from the given position to the next
 * voxel in its path. The position, the corresponding voxel
 * coordinate, and the distance to these is returned.
 *
 * @param position The position of the ray.
 * @param voxel_coord The stepped-to voxel coordinate.
 * @param steps_to_next The number of steps needed to get to
 * next voxel in each axis..
 * @param step If we step along an axis, this is which way.
 * @param delta The relative weighting of stepping along each
 * axis. Higher delta means "more" steps to get to next voxel
 * along that axis.
 * @param direction The direction of the ray.
 * @param distance The distance the ray travelled in the step.
 */
inline static void step_to_next_voxel_position(
    IN OUT f32v3& position,
    IN OUT hvox::VoxelWorldPosition& voxel_coord,
    IN OUT f32v3&                    steps_to_next,
    const hvox::VoxelWorldPosition&  step,
    const f32v3&                     delta,
    f32v3                            direction,
    IN OUT f32&                      distance
) {
    // Assume direction here is normalised.

    distance = 0.0f;

    if (steps_to_next.x < steps_to_next.y) {
        if (steps_to_next.x < steps_to_next.z) {
            if (steps_to_next.x < delta.x) {
                position += steps_to_next.x * direction;
                distance += steps_to_next.x;
            } else {
                position += delta.x * direction;
                distance += delta.x;
            }

            voxel_coord.x   += step.x;
            steps_to_next.x += delta.x;
        } else {
            if (steps_to_next.z < delta.z) {
                position += steps_to_next.z * direction;
                distance += steps_to_next.z;
            } else {
                position += delta.z * direction;
                distance += delta.z;
            }

            voxel_coord.z   += step.z;
            steps_to_next.z += delta.z;
        }
    } else {
        if (steps_to_next.y < steps_to_next.z) {
            if (steps_to_next.y < delta.y) {
                position += steps_to_next.y * direction;
                distance += steps_to_next.y;
            } else {
                position += delta.y * direction;
                distance += delta.y;
            }

            voxel_coord.y   += step.y;
            steps_to_next.y += delta.y;
        } else {
            if (steps_to_next.z < delta.z) {
                position += steps_to_next.z * direction;
                distance += steps_to_next.z;
            } else {
                position += delta.z * direction;
                distance += delta.z;
            }

            voxel_coord.z   += step.z;
            steps_to_next.z += delta.z;
        }
    }
}

bool hvox::Ray::cast_to_voxel(
    f32v3                       start,
    f32v3                       direction,
    hmem::WeakHandle<ChunkGrid> chunk_handle,
    Voxel                       target_voxel,
    ui32                        max_steps,
    OUT VoxelWorldPosition&     position,
    OUT f32&                    distance,
    OUT hmem::WeakHandle<Chunk>* chunk /*= nullptr*/
) {
    return cast_to_voxel(
        start,
        direction,
        chunk_handle,
        { [target_voxel](const Voxel& test) {
            return target_voxel == test;
        } },
        max_steps,
        position,
        distance,
        chunk
    );
}

bool hvox::Ray::cast_to_voxel(
    f32v3                       start,
    f32v3                       direction,
    hmem::WeakHandle<ChunkGrid> chunk_handle,
    VoxelTest                   voxel_is_target,
    ui32                        max_steps,
    OUT VoxelWorldPosition&     position,
    OUT f32&                    distance,
    OUT hmem::WeakHandle<Chunk>* chunk /*= nullptr*/
) {
    auto chunk_grid = chunk_handle.lock();

    if (chunk_grid == nullptr) return false;

    VoxelWorldPosition step
        = VoxelWorldPosition{ sign(direction.x), sign(direction.y), sign(direction.z) };
    f32v3 delta = f32v3{
        static_cast<f32>(step.x) / direction.x,
        static_cast<f32>(step.y) / direction.y,
        static_cast<f32>(step.z) / direction.z,
    };
    f32v3 steps_to_next = f32v3{ min_coeff_to_int(start.x, direction.x),
                                 min_coeff_to_int(start.y, direction.y),
                                 min_coeff_to_int(start.z, direction.z) };
    position            = voxel_world_position(start);
    distance            = 0.0f;

    ui32  steps = 0;
    Voxel voxel{};

    ChunkGridPosition old_chunk_pos = chunk_grid_position(position);

    auto chunk_tmp = chunk_grid->chunk(old_chunk_pos);

    if (chunk_tmp == nullptr) return false;

    do {
        step_to_next_voxel_position(
            start, position, steps_to_next, step, delta, direction, distance
        );

        ChunkGridPosition new_chunk_pos = chunk_grid_position(position);

        if (new_chunk_pos != old_chunk_pos) {
            chunk_tmp = chunk_grid->chunk(new_chunk_pos);

            // TODO(Matthew): do we want to allow "seeing through" unloaded chunks?
            if (chunk_tmp == nullptr) return false;
        }

        old_chunk_pos = new_chunk_pos;

        std::shared_lock<std::shared_mutex> lock;
        auto                                chunk_voxels = chunk_tmp->voxels.get(lock);

        auto idx = voxel_index(voxel_chunk_position(position));

        voxel = chunk_voxels[idx];

        if (voxel_is_target(voxel)) {
            if (chunk) *chunk = chunk_tmp;

            return true;
        }
    } while (++steps < max_steps);

    return false;
}

bool hvox::Ray::cast_to_voxel_before(
    f32v3                       start,
    f32v3                       direction,
    hmem::WeakHandle<ChunkGrid> chunk_handle,
    Voxel                       target_voxel,
    ui32                        max_steps,
    OUT VoxelWorldPosition&     position,
    OUT f32&                    distance,
    OUT hmem::WeakHandle<Chunk>* chunk /*= nullptr*/
) {
    return cast_to_voxel_before(
        start,
        direction,
        chunk_handle,
        { [target_voxel](const Voxel& test) {
            return target_voxel == test;
        } },
        max_steps,
        position,
        distance,
        chunk
    );
}

bool hvox::Ray::cast_to_voxel_before(
    f32v3                       start,
    f32v3                       direction,
    hmem::WeakHandle<ChunkGrid> chunk_handle,
    VoxelTest                   voxel_is_target,
    ui32                        max_steps,
    OUT VoxelWorldPosition&     position,
    OUT f32&                    distance,
    OUT hmem::WeakHandle<Chunk>* chunk /*= nullptr*/
) {
    auto chunk_grid = chunk_handle.lock();

    if (chunk_grid == nullptr) return false;

    VoxelWorldPosition step
        = VoxelWorldPosition{ sign(direction.x), sign(direction.y), sign(direction.z) };
    f32v3 delta = f32v3{
        static_cast<f32>(step.x) / direction.x,
        static_cast<f32>(step.y) / direction.y,
        static_cast<f32>(step.z) / direction.z,
    };
    f32v3              steps_to_next  = f32v3{ min_coeff_to_int(start.x, direction.x),
                                 min_coeff_to_int(start.y, direction.y),
                                 min_coeff_to_int(start.z, direction.z) };
    VoxelWorldPosition voxel_position = voxel_world_position(start);
    position                          = voxel_position;
    distance                          = 0.0f;

    ui32  steps = 0;
    Voxel voxel{};

    ChunkGridPosition old_chunk_pos = chunk_grid_position(voxel_position);

    auto chunk_tmp = chunk_grid->chunk(old_chunk_pos);

    if (chunk_tmp == nullptr) return false;

    do {
        f32 step_distance = 0.0f;
        step_to_next_voxel_position(
            start, voxel_position, steps_to_next, step, delta, direction, step_distance
        );

        ChunkGridPosition new_chunk_pos = chunk_grid_position(voxel_position);

        if (new_chunk_pos != old_chunk_pos) {
            chunk_tmp = chunk_grid->chunk(new_chunk_pos);

            // TODO(Matthew): do we want to allow "seeing through" unloaded chunks?
            if (chunk_tmp == nullptr) return false;
        }

        old_chunk_pos = new_chunk_pos;

        std::shared_lock<std::shared_mutex> lock;
        auto                                chunk_voxels = chunk_tmp->voxels.get(lock);

        auto idx = voxel_index(voxel_chunk_position(voxel_position));

        voxel = chunk_voxels[idx];

        if (voxel_is_target(voxel)) {
            if (chunk) *chunk = chunk_tmp;

            return true;
        }

        distance += step_distance;
        position = voxel_position;
    } while (++steps < max_steps);

    return false;
}

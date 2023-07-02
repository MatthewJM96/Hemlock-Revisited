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
 * block in its path. The position, the corresponding block
 * coordinate, and the distance to these is returned.
 *
 * @param position The position of the ray.
 * @param block_coord The stepped-to block coordinate.
 * @param steps_to_next The number of steps needed to get to
 * next block in each axis..
 * @param step If we step along an axis, this is which way.
 * @param delta The relative weighting of stepping along each
 * axis. Higher delta means "more" steps to get to next block
 * along that axis.
 * @param direction The direction of the ray.
 * @param distance The distance the ray travelled in the step.
 */
inline static void step_to_next_block_position(
    IN OUT f32v3& position,
    IN OUT hvox::BlockWorldPosition& block_coord,
    IN OUT f32v3&                    steps_to_next,
    const hvox::BlockWorldPosition&  step,
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

            block_coord.x   += step.x;
            steps_to_next.x += delta.x;
        } else {
            if (steps_to_next.z < delta.z) {
                position += steps_to_next.z * direction;
                distance += steps_to_next.z;
            } else {
                position += delta.z * direction;
                distance += delta.z;
            }

            block_coord.z   += step.z;
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

            block_coord.y   += step.y;
            steps_to_next.y += delta.y;
        } else {
            if (steps_to_next.z < delta.z) {
                position += steps_to_next.z * direction;
                distance += steps_to_next.z;
            } else {
                position += delta.z * direction;
                distance += delta.z;
            }

            block_coord.z   += step.z;
            steps_to_next.z += delta.z;
        }
    }
}

bool hvox::Ray::cast_to_block(
    f32v3                       start,
    f32v3                       direction,
    hmem::WeakHandle<ChunkGrid> chunk_handle,
    Block                       target_block,
    ui32                        max_steps,
    OUT BlockWorldPosition&     position,
    OUT f32&                    distance
) {
    return cast_to_block(
        start,
        direction,
        chunk_handle,
        { [target_block](const Block& test) {
            return target_block == test;
        } },
        max_steps,
        position,
        distance
    );
}

bool hvox::Ray::cast_to_block(
    f32v3                       start,
    f32v3                       direction,
    hmem::WeakHandle<ChunkGrid> chunk_handle,
    BlockTest                   block_is_target,
    ui32                        max_steps,
    OUT BlockWorldPosition&     position,
    OUT f32&                    distance
) {
    auto chunk_grid = chunk_handle.lock();

    if (chunk_grid == nullptr) return false;

    BlockWorldPosition step
        = BlockWorldPosition{ sign(direction.x), sign(direction.y), sign(direction.z) };
    f32v3 delta = f32v3{
        static_cast<f32>(step.x) / direction.x,
        static_cast<f32>(step.y) / direction.y,
        static_cast<f32>(step.z) / direction.z,
    };
    f32v3 steps_to_next = f32v3{ min_coeff_to_int(start.x, direction.x),
                                 min_coeff_to_int(start.y, direction.y),
                                 min_coeff_to_int(start.z, direction.z) };
    position            = block_world_position(start);
    distance            = 0.0f;

    ui32  steps = 0;
    Block block{};

    ChunkGridPosition old_chunk_pos = chunk_grid_position(position);

    auto chunk = chunk_grid->chunk(old_chunk_pos);

    if (chunk == nullptr) return false;

    do {
        step_to_next_block_position(
            start, position, steps_to_next, step, delta, direction, distance
        );

        ChunkGridPosition new_chunk_pos = chunk_grid_position(position);

        if (new_chunk_pos != old_chunk_pos) {
            chunk = chunk_grid->chunk(new_chunk_pos);

            // TODO(Matthew): do we want to allow "seeing through" unloaded chunks?
            if (chunk == nullptr) return false;
        }

        old_chunk_pos = new_chunk_pos;

        std::shared_lock lock(chunk->blocks_mutex);

        auto idx = block_index(block_chunk_position(position));

        block = chunk->blocks[idx];

        if (block_is_target(block)) return true;
    } while (++steps < max_steps);

    return false;
}

bool hvox::Ray::cast_to_block_before(
    f32v3                       start,
    f32v3                       direction,
    hmem::WeakHandle<ChunkGrid> chunk_handle,
    Block                       target_block,
    ui32                        max_steps,
    OUT BlockWorldPosition&     position,
    OUT f32&                    distance
) {
    return cast_to_block_before(
        start,
        direction,
        chunk_handle,
        { [target_block](const Block& test) {
            return target_block == test;
        } },
        max_steps,
        position,
        distance
    );
}

bool hvox::Ray::cast_to_block_before(
    f32v3                       start,
    f32v3                       direction,
    hmem::WeakHandle<ChunkGrid> chunk_handle,
    BlockTest                   block_is_target,
    ui32                        max_steps,
    OUT BlockWorldPosition&     position,
    OUT f32&                    distance
) {
    auto chunk_grid = chunk_handle.lock();

    if (chunk_grid == nullptr) return false;

    BlockWorldPosition step
        = BlockWorldPosition{ sign(direction.x), sign(direction.y), sign(direction.z) };
    f32v3 delta = f32v3{
        static_cast<f32>(step.x) / direction.x,
        static_cast<f32>(step.y) / direction.y,
        static_cast<f32>(step.z) / direction.z,
    };
    f32v3              steps_to_next  = f32v3{ min_coeff_to_int(start.x, direction.x),
                                 min_coeff_to_int(start.y, direction.y),
                                 min_coeff_to_int(start.z, direction.z) };
    BlockWorldPosition block_position = block_world_position(start);
    position                          = block_position;
    distance                          = 0.0f;

    ui32  steps = 0;
    Block block{};

    ChunkGridPosition old_chunk_pos = chunk_grid_position(block_position);

    auto chunk = chunk_grid->chunk(old_chunk_pos);

    if (chunk == nullptr) return false;

    do {
        f32 step_distance = 0.0f;
        step_to_next_block_position(
            start, block_position, steps_to_next, step, delta, direction, step_distance
        );

        ChunkGridPosition new_chunk_pos = chunk_grid_position(block_position);

        if (new_chunk_pos != old_chunk_pos) {
            chunk = chunk_grid->chunk(new_chunk_pos);

            // TODO(Matthew): do we want to allow "seeing through" unloaded chunks?
            if (chunk == nullptr) return false;
        }

        old_chunk_pos = new_chunk_pos;

        std::shared_lock lock(chunk->blocks_mutex);

        auto idx = block_index(block_chunk_position(block_position));

        block = chunk->blocks[idx];

        if (block_is_target(block)) return true;

        distance += step_distance;
        position = block_position;
    } while (++steps < max_steps);

    return false;
}

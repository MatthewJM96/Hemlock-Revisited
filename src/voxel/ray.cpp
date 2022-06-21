#include "stdafx.h"

#include "voxel/chunk/grid.h"
#include "voxel/ray.h"

/**
 * @brief Steps a ray from the given position to the next
 * block in its path. The position, the corresponding block
 * coordinate, and the distance to these is returned.
 *
 * @param position The position of the ray.
 * @param block_coord The stepped-to block coordinate.
 * @param direction The direction of the ray.
 * @param distance The distance the ray travelled in the step.
 */
inline static
void step_to_next_block_position(
    IN OUT                      f32v3& position,
    IN OUT   hvox::BlockWorldPosition& block_coord,
                                 f32v3 direction,
       OUT                        f32& distance
) {
    // Assume direction here is normalised.

    distance = 0.0f;

    size_t min_axis             = std::numeric_limits<size_t>::max();
    f32    min_weighting        = std::numeric_limits<f32>::max();
    f32    candidate_weighting  = 0.0f;

    // X
    if (direction.x < 0.0f) {
        min_axis        = 0;
        min_weighting   = (glm::floor(position.x) - position.x) / direction.x;
    } else if (direction.x > 0.0f) {
        min_axis        = 0;
        min_weighting   = (glm::ceil(position.x) - position.x) / direction.x;
    }

    // Y
    if (direction.y < 0.0f) {
        candidate_weighting = (glm::floor(position.y) - position.y) / direction.y;

        if (candidate_weighting < min_weighting) {
            min_axis        = 1;
            min_weighting   = candidate_weighting;
        }
    } else if (direction.y > 0.0f) {
        candidate_weighting = (glm::ceil(position.y) - position.y) / direction.y;

        if (candidate_weighting < min_weighting) {
            min_axis        = 1;
            min_weighting   = candidate_weighting;
        }
    }

    // Z
    if (direction.z < 0.0f) {
        candidate_weighting = (glm::floor(position.z) - position.z) / direction.z;

        if (candidate_weighting < min_weighting) {
            min_axis        = 2;
            min_weighting   = candidate_weighting;
        }
    } else if (direction.z > 0.0f) {
        candidate_weighting = (glm::ceil(position.z) - position.z) / direction.z;

        if (candidate_weighting < min_weighting) {
            min_axis        = 2;
            min_weighting   = candidate_weighting;
        }
    }

    block_coord[min_axis] += 1;

    f32v3 old_position = position;
    position += min_weighting * direction;

    f32v3 move = old_position - position;
    distance = glm::sqrt(glm::dot(move, move));
}

bool hvox::Ray::cast_to_block(      f32v3 start, 
                                    f32v3 direction,
              hmem::WeakHandle<ChunkGrid> chunk_handle,
                                    Block target_block,
                                     ui32 max_steps,
                  OUT BlockWorldPosition& position,
                                 OUT f32& distance      )
{
    return cast_to_block(
        start,
        direction,
        chunk_handle,
        {[target_block](const Block& test) { return target_block == test; }},
        max_steps,
        position,
        distance
    );
}

bool hvox::Ray::cast_to_block(      f32v3 start,
                                    f32v3 direction,
              hmem::WeakHandle<ChunkGrid> chunk_handle,
                                BlockTest block_is_target,
                                     ui32 max_steps,
                  OUT BlockWorldPosition& position,
                                 OUT f32& distance      )
{
    auto chunk_grid = chunk_handle.lock();

    if (chunk_grid == nullptr) return false;

    position = block_world_position(start);
    distance = 0.0f;

    ui32 steps = 0;
    Block block{};

    do {
        f32 step_size;
        step_to_next_block_position(start, position, direction, step_size);

        distance += step_size;

        auto chunk = chunk_grid->chunk(chunk_grid_position(position));

        std::shared_lock lock(chunk->blocks_mutex);

        auto idx = block_index(
            block_chunk_position(position)
        );

        block = chunk->blocks[idx];

        if (block_is_target(block)) return true;
    } while (++steps < max_steps);

    return false;
}

bool hvox::Ray::cast_to_block_before( f32v3 start, 
                                      f32v3 direction,
                hmem::WeakHandle<ChunkGrid> chunk_handle,
                                      Block target_block,
                                       ui32 max_steps,
                    OUT BlockWorldPosition& position,
                                   OUT f32& distance      )
{
    return cast_to_block_before(
        start,
        direction,
        chunk_handle,
        {[target_block](const Block& test) { return target_block == test; }},
        max_steps,
        position,
        distance
    );
}

bool hvox::Ray::cast_to_block_before( f32v3 start,
                                      f32v3 direction,
                hmem::WeakHandle<ChunkGrid> chunk_handle,
                                  BlockTest block_is_target,
                                       ui32 max_steps,
                    OUT BlockWorldPosition& position,
                                   OUT f32& distance      )
{
    auto chunk_grid = chunk_handle.lock();

    if (chunk_grid == nullptr) return false;

    BlockWorldPosition block_position = block_world_position(start);
    distance = 0.0f;

    ui32 steps = 0;
    Block block{};

    do {
        f32 step_size;
        step_to_next_block_position(start, block_position, direction, step_size);

        auto chunk = chunk_grid->chunk(chunk_grid_position(block_position));

        std::shared_lock lock(chunk->blocks_mutex);

        auto idx = block_index(
            block_chunk_position(block_position)
        );

        block = chunk->blocks[idx];

        if (block_is_target(block)) return true;

        distance += step_size;
        position  = block_position;
    } while (++steps < max_steps);

    return false;
}

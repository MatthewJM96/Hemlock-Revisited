#include "stdafx.h"

#include "voxel/chunk/grid.h"
#include "voxel/ray.h"

inline static void step_to_next_block_position(IN OUT f32v3& position, f32v3 direction, OUT f32& distance) {
    // Assume direction here is normalised, blocks are defined on the integer boundaries.
    // TODO(Matthew): I think the way block is defined, and the meshers work, blocks are rendered on half-integer boundaries.

    distance = 0.0f;

    f32 min_orthogonal_weighting = std::numeric_limits<f32>::max();
    f32 candidate_orthogonal_weighting = 0.0f;

    // X
    if (direction.x < 0.0f) {
        min_orthogonal_weighting = (glm::floor(position.x) - position.x) / direction.x;
    } else if (direction.x > 0.0f) {
        min_orthogonal_weighting = (glm::ceil(position.x) - position.x) / direction.x;
    }


    // Y
    if (direction.y < 0.0f) {
        candidate_orthogonal_weighting = (glm::floor(position.y) - position.y) / direction.y;

        if (candidate_orthogonal_weighting < min_orthogonal_weighting) min_orthogonal_weighting = candidate_orthogonal_weighting;
    } else if (direction.y > 0.0f) {
        candidate_orthogonal_weighting = (glm::ceil(position.y) - position.y) / direction.y;

        if (candidate_orthogonal_weighting < min_orthogonal_weighting) min_orthogonal_weighting = candidate_orthogonal_weighting;
    }

    // Z
    if (direction.z < 0.0f) {
        candidate_orthogonal_weighting = (glm::floor(position.z) - position.z) / direction.z;

        if (candidate_orthogonal_weighting < min_orthogonal_weighting) min_orthogonal_weighting = candidate_orthogonal_weighting;
    } else if (direction.z > 0.0f) {
        candidate_orthogonal_weighting = (glm::ceil(position.z) - position.z) / direction.z;

        if (candidate_orthogonal_weighting < min_orthogonal_weighting) min_orthogonal_weighting = candidate_orthogonal_weighting;
    }

    f32v3 old_position = position;
    position += min_orthogonal_weighting * direction;

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

    // TODO(Matthew): implement

    (void)start;
    (void)direction;
    (void)block_is_target;
    (void)max_steps;
    (void)position;
    (void)distance;

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

bool hvox::Ray::cast_to_block_before( f32v3start,
                                      f32v3 direction,
                hmem::WeakHandle<ChunkGrid> chunk_handle,
                                  BlockTest block_is_target,
                                       ui32 max_steps,
                    OUT BlockWorldPosition& position,
                                   OUT f32& distance      )
{
    auto chunk_grid = chunk_handle.lock();

    if (chunk_grid == nullptr) return false;

    // TODO(Matthew): implement

    (void)start;
    (void)direction;
    (void)block_is_target;
    (void)max_steps;
    (void)position;
    (void)distance;

    return false;
}

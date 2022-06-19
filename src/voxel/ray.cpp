#include "stdafx.h"

#include "voxel/ray.h"

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

    return false;
}

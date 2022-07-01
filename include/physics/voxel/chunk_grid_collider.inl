#include "voxel/chunk/grid.h"

template <hphys::VoxelShapeEvaluator ShapeEvaluator>
void hphys::ChunkGridCollider::determine_candidate_colliding_voxels(
      AnchoredComponent ac,
       DynamicComponent dc,
    CollidableComponent cc,
       btCompoundShape* voxels
) {
    const ShapeEvaluator shape_evaluator = {};

    auto chunk_grid = ac.chunk_grid.lock();
    // TODO(Matthew): too strict?
    assert(chunk_grid != nullptr);

    // TODO(Matthew): This should be set somehow to reflect a reasonable amount
    //                of time a physics collision handling will occur over
    //                before returning to this function.
    //                  If this is not possible, we need to consider the idea
    //                  of extending Bullet with a VoxelWorld concept to allow
    //                  ray casting to be used to choose which blocks to include
    //                  in resulting compound shape.
    const f32 dt = 1.0f;

    btVector3 min_aabb, max_aabb;
    cc.shape.getAabb(btTransform::getIdentity(), min_aabb, max_aabb);

    f32v3 max_ds = glm::abs(dc.velocity) * dt;

    min_aabb -= btVector3(max_ds.x, max_ds.y, max_ds.z);
    max_aabb += btVector3(max_ds.x, max_ds.y, max_ds.z);

    auto min_local_coord = hvox::block_world_position(f32v3(min_aabb.x(), min_aabb.y(), min_aabb.z()));
    auto max_local_coord = hvox::block_world_position(f32v3(max_aabb.x(), max_aabb.y(), max_aabb.z()));

    auto min_world_entity_coord = ac.position + hvox::EntityWorldPosition{
        static_cast<hvox::EntityWorldPositionCoord>(min_local_coord.x) << 32,
        static_cast<hvox::EntityWorldPositionCoord>(min_local_coord.y) << 32,
        static_cast<hvox::EntityWorldPositionCoord>(min_local_coord.z) << 32
    };
    auto max_world_entity_coord = ac.position + hvox::EntityWorldPosition{
        static_cast<hvox::EntityWorldPositionCoord>(max_local_coord.x) << 32,
        static_cast<hvox::EntityWorldPositionCoord>(max_local_coord.y) << 32,
        static_cast<hvox::EntityWorldPositionCoord>(max_local_coord.z) << 32
    };

    auto min_world_block_coord = hvox::BlockWorldPosition{
        static_cast<hvox::BlockWorldPositionCoord>(min_world_entity_coord.x >> 32),
        static_cast<hvox::BlockWorldPositionCoord>(min_world_entity_coord.y >> 32),
        static_cast<hvox::BlockWorldPositionCoord>(min_world_entity_coord.z >> 32)
    };
    auto max_world_block_coord = hvox::BlockWorldPosition{
        static_cast<hvox::BlockWorldPositionCoord>(max_world_entity_coord.x >> 32),
        static_cast<hvox::BlockWorldPositionCoord>(max_world_entity_coord.y >> 32),
        static_cast<hvox::BlockWorldPositionCoord>(max_world_entity_coord.z >> 32)
    };

    auto new_chunk_coord = hvox::chunk_grid_position(
        hvox::BlockWorldPosition{
            min_world_block_coord.x,
            min_world_block_coord.y,
            min_world_block_coord.z
        }
    );
    auto old_chunk_coord = new_chunk_coord;

    auto chunk = chunk_grid->chunk(new_chunk_coord);
    std::shared_lock lock(chunk->blocks_mutex);
    for (auto x = min_world_block_coord.x; x < max_world_block_coord.x; ++x) {
        for (auto y = min_world_block_coord.y; y < max_world_block_coord.y; ++y) {
            for (auto z = min_world_block_coord.z; z < max_world_block_coord.z; ++z) {
                new_chunk_coord = hvox::chunk_grid_position({x,y,z});

                if (old_chunk_coord != new_chunk_coord) {
                    chunk           = chunk_grid->chunk(new_chunk_coord);
                    lock            = std::shared_lock(chunk->blocks_mutex);
                    old_chunk_coord = new_chunk_coord;
                }
                
                auto block_idx = hvox::block_index(hvox::block_chunk_position({x,y,z}));
                auto block     = chunk->blocks[block_idx];

                btTransform transform = btTransform::getIdentity();
                btCollisionShape* shape = shape_evaluator(block, transform);
                if (shape)
                    voxels->addChildShape(transform, shape);
            }
        }
    }
}

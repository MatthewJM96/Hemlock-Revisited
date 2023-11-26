#include "voxel/chunk/grid.h"

template <hphys::VoxelShapeEvaluator ShapeEvaluator>
bool hphys::ChunkGridCollider::determine_candidate_colliding_voxels(
    AnchoredComponent   ac,
    DynamicComponent    dc,
    CollidableComponent cc,
    btCompoundShape*    voxels
) {
    const ShapeEvaluator shape_evaluator = {};

    auto chunk_grid = ac.chunk_grid.lock();
    if (chunk_grid == nullptr) return false;

    // TODO(Matthew): Can we somehow take advantage of greedy meshing for these
    // calculations?
    //                  => Consider fewer objects in each chunk.

    // TODO(Matthew): This should be set somehow to reflect a reasonable amount
    //                of time a physics collision handling will occur over
    //                before returning to this function.
    //                  If this is not possible, we need to consider the idea
    //                  of extending Bullet with a VoxelWorld concept to allow
    //                  ray casting to be used to choose which blocks to include
    //                  in resulting compound shape.
    const f32 dt = 1.0f;

    btVector3 min_aabb, max_aabb;
    cc.shape->getAabb(btTransform::getIdentity(), min_aabb, max_aabb);

    f32v3 max_ds = glm::abs(dc.velocity) * dt;

    min_aabb -= btVector3(max_ds.x, max_ds.y, max_ds.z);
    max_aabb += btVector3(max_ds.x, max_ds.y, max_ds.z);

    auto min_local_coord
        = hvox::block_world_position(f32v3(min_aabb.x(), min_aabb.y(), min_aabb.z()));
    auto max_local_coord
        = hvox::block_world_position(f32v3(max_aabb.x(), max_aabb.y(), max_aabb.z()));

    auto min_world_entity_coord
        = ac.position
          + hvox::EntityWorldPosition{
                static_cast<hvox::EntityWorldPositionCoord>(min_local_coord.x) << 32,
                static_cast<hvox::EntityWorldPositionCoord>(min_local_coord.y) << 32,
                static_cast<hvox::EntityWorldPositionCoord>(min_local_coord.z) << 32
            };
    auto max_world_entity_coord
        = ac.position
          + hvox::EntityWorldPosition{
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

    btVector3 half_range = btVector3{ (static_cast<btScalar>(max_world_block_coord.x)
                                       - static_cast<btScalar>(min_world_block_coord.x))
                                          / 2.0f,
                                      (static_cast<btScalar>(max_world_block_coord.y)
                                       - static_cast<btScalar>(min_world_block_coord.y))
                                          / 2.0f,
                                      (static_cast<btScalar>(max_world_block_coord.z)
                                       - static_cast<btScalar>(min_world_block_coord.z))
                                          / 2.0f };

    auto new_chunk_coord = hvox::chunk_grid_position(min_world_block_coord);
    auto old_chunk_coord = new_chunk_coord;

    bool any_collidable = false;

    hmem::Handle<hvox::Chunk>           chunk;
    std::shared_lock<std::shared_mutex> lock;
    const hvox::Block*                  blocks;

    for (auto x = min_world_block_coord.x; x < max_world_block_coord.x; ++x) {
        for (auto y = min_world_block_coord.y; y < max_world_block_coord.y; ++y) {
            for (auto z = min_world_block_coord.z; z < max_world_block_coord.z; ++z) {
                new_chunk_coord = hvox::chunk_grid_position({ x, y, z });

                if (chunk == nullptr || (old_chunk_coord != new_chunk_coord)) {
                    // TODO(Matthew): should query that a chunk isn't just in
                    // existence,
                    //                but that it also has generated.
                    // Considering block in new chunk, get handle on that new
                    // chunk and check it at least exists.
                    chunk = chunk_grid->chunk(new_chunk_coord);
                    if (chunk == nullptr) continue;

                    // Chunk exists, get lock on blocks.
                    blocks = chunk->blocks.get(lock);

                    old_chunk_coord = new_chunk_coord;
                }

                auto block_idx
                    = hvox::block_index(hvox::block_chunk_position({ x, y, z }));
                auto block = blocks[block_idx];

                btTransform       transform = btTransform::getIdentity();
                btCollisionShape* shape     = shape_evaluator(block, transform);
                if (shape) {
                    // TODO(Matthew): In general, we need to make sure we are getting
                    // the
                    //                coordinate systems of the colliding entity and
                    //                the patch of the chunk grid voxels aligned
                    //                correctly - at whichever point in the collision
                    //                calculation process bullet wants that.
                    //                    Probably here we want to just subtract the
                    //                    average like so:
                    transform.getOrigin() += btVector3{
                        static_cast<btScalar>(x) - half_range.x()
                            - static_cast<btScalar>(min_world_block_coord.x),
                        static_cast<btScalar>(y) - half_range.y()
                            - static_cast<btScalar>(min_world_block_coord.y),
                        static_cast<btScalar>(z) - half_range.z()
                            - static_cast<btScalar>(min_world_block_coord.z)
                    };
                    voxels->addChildShape(transform, shape);

                    any_collidable = true;
                }
            }
        }
    }

    return any_collidable;
}

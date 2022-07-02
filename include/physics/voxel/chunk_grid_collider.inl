#include "voxel/chunk/grid.h"

template <hphys::VoxelShapeEvaluator ShapeEvaluator>
bool hphys::ChunkGridCollider::determine_candidate_colliding_voxels(
      AnchoredComponent ac,
       DynamicComponent dc,
     RigidBodyComponent rbc,
       btCompoundShape* voxels
) {
    const ShapeEvaluator shape_evaluator = {};

    auto chunk_grid = ac.chunk_grid.lock();
    if (chunk_grid == nullptr)
        return false;

    // TODO(Matthew): This should be set somehow to reflect a reasonable amount
    //                of time a physics collision handling will occur over
    //                before returning to this function.
    //                  If this is not possible, we need to consider the idea
    //                  of extending Bullet with a VoxelWorld concept to allow
    //                  ray casting to be used to choose which blocks to include
    //                  in resulting compound shape.
    const f32 dt = 1.0f;

    // TODO(Matthew): Right now this function assumes that we get the world transform
    //                for the aabb from the body. We should check we are happy with this
    //                as if we operate in large enough spaces, we will find that the
    //                resulting behaviour becomes less ideal at long distances. We might
    //                get away with it here as we round out the decimal component, but
    //                this assumption has implications elsewhere.

    btVector3 min_aabb, max_aabb;
    rbc.body->getAabb(min_aabb, max_aabb);

    f32v3 max_ds = glm::abs(dc.velocity) * dt;

    min_aabb -= btVector3(max_ds.x, max_ds.y, max_ds.z);
    max_aabb += btVector3(max_ds.x, max_ds.y, max_ds.z);

    hvox::BlockWorldPosition min_world_block_coord = hvox::block_world_position(f32v3(min_aabb.x(), min_aabb.y(), min_aabb.z()));
    hvox::BlockWorldPosition max_world_block_coord = hvox::block_world_position(glm::ceil(f32v3(max_aabb.x(), max_aabb.y(), max_aabb.z())));

    auto new_chunk_coord = hvox::chunk_grid_position(
        hvox::BlockWorldPosition{
            min_world_block_coord.x,
            min_world_block_coord.y,
            min_world_block_coord.z
        }
    );
    auto old_chunk_coord = new_chunk_coord;

    bool any_collidable = false;

    auto chunk = chunk_grid->chunk(new_chunk_coord);
    // TODO(Matthew): we don't want to fall through unloaded chunks.
    if (chunk == nullptr)
        return false;

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
                if (shape) {
                    transform.getOrigin() += btVector3{
                        static_cast<btScalar>(x) + 0.5f,
                        static_cast<btScalar>(y) + 0.5f,
                        static_cast<btScalar>(z) + 0.5f
                    };
                    voxels->addChildShape(transform, shape);

                    any_collidable = true;
                }
            }
        }
    }

    return any_collidable;
}

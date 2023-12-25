#ifndef __hemlock_physics_voxel_chunk_grid_collider_hpp
#define __hemlock_physics_voxel_chunk_grid_collider_hpp

#include "physics/common_components.hpp"
#include "physics/voxel/anchored_component.hpp"

namespace hemlock {
    namespace physics {
        /**
         * @brief Defines a struct whose opeartor() determines the shape of a voxel.
         *
         * The function returns a pointer to an appropriate btCollisionShape if the
         * voxel can be collided with, otherwise nullptr.
         */
        template <typename EvaluatorCandidate>
        concept VoxelShapeEvaluator
            = requires (EvaluatorCandidate e, hvox::Voxel b, btTransform& t) {
                  {
                      e.operator()(b, t)
                      } -> std::same_as<btCollisionShape*>;
              };

        namespace ChunkGridCollider {
            template <hphys::VoxelShapeEvaluator ShapeEvaluator>
            bool determine_candidate_colliding_voxels(
                AnchoredComponent   ac,
                DynamicComponent    dc,
                CollidableComponent cc,
                btCompoundShape*    colliders
            );
        }
    }  // namespace physics
}  // namespace hemlock
namespace hphys = hemlock::physics;

#include "chunk_grid_collider.inl"

#endif  // __hemlock_physics_voxel_chunk_grid_collider_hpp

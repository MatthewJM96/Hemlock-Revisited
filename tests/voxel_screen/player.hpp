#ifndef __hemlock_tests_voxel_screen_player_hpp
#define __hemlock_tests_voxel_screen_player_hpp

#include "voxel/coordinate_system.h"

#include "physics.hpp"

namespace hemlock {
    namespace test {
        namespace voxel_screen {
            struct PlayerData {
                hphys::CollidableComponent cc;
                hphys::AnchoredComponent   ac;
                hphys::DynamicComponent    dc;
                btRigidBody*               body;
            };

            void setup_player(PlayerData& player, PhysicsData& phys, const hcam::BasicFirstPersonCamera& camera) {
                player.ac.position   = hvox::EntityWorldPosition{0, static_cast<hvox::EntityWorldPositionCoord>(60) << 32, 0};
                player.ac.chunk_grid = m_chunk_grid;
                player.cc.shape = new btCompoundShape();
                player.cc.shape->addChildShape(btTransform::getIdentity(), new btBoxShape(btVector3{0.5f, 1.5f, 0.5f}));
                // TODO(Matthew): update this.
                player.dc.velocity   = f32v3(2.0f);

                btQuaternion rotation;
                rotation.setEulerZYX(0.0f, 1.0f, 0.0f);
                btVector3 position = btVector3(camera.position().x, camera.position().y, camera.position().z);
                btDefaultMotionState* motion_state = new btDefaultMotionState(btTransform(rotation, position));
                btVector3 inertia;
                btScalar mass = 80.0f;
                player.cc.shape->calculateLocalInertia(mass, inertia);
                btRigidBody::btRigidBodyConstructionInfo body_info = btRigidBody::btRigidBodyConstructionInfo(mass, motion_state, player.cc.shape, inertia);
                body_info.m_restitution = 0.0f;
                body_info.m_friction = 1000.0f;
                player.body = new btRigidBody(body_info);
                player.body->setAngularFactor(0.0f);
                phys.world->addRigidBody(player.body);
            }
        }
    }
}
namespace htest = hemlock::test;

#endif // __hemlock_tests_voxel_screen_player_hpp

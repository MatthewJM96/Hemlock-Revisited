#ifndef __hemlock_tests_voxel_screen_physics_hpp
#define __hemlock_tests_voxel_screen_physics_hpp

#include "tests/voxel_screen/terrain/physics.hpp"

namespace hemlock {
    namespace test {
        namespace voxel_screen {
            struct PhysicsData {
                btDbvtBroadphase*                       broadphase;
                btDefaultCollisionConfiguration*        collision_config;
                btCollisionDispatcher*                  dispatcher;
                btSequentialImpulseConstraintSolver*    solver;
                btDiscreteDynamicsWorld*                world;
            };

            void setup_physics(
                                 PhysicsData& phys,
                hcam::BasicFirstPersonCamera& camera,
                             hg::GLSLProgram* line_shader
             ) {
                phys.broadphase       = new btDbvtBroadphase();
                phys.collision_config = new btDefaultCollisionConfiguration();
                phys.dispatcher       = new btCollisionDispatcher(phys.collision_config);
                phys.solver           = new btSequentialImpulseConstraintSolver();
                phys.world            = new btDiscreteDynamicsWorld(phys.dispatcher, phys.broadphase, phys.solver, phys.collision_config);
                phys.world->setGravity(btVector3(0, 0, 0));

                phys.world->setDebugDrawer(new VoxelPhysDrawer(&camera, line_shader));
                phys.world->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
            }
        }
    }
}
namespace htest = hemlock::test;

#endif // __hemlock_tests_voxel_screen_physics_hpp

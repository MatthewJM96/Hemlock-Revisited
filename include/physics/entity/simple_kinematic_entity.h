#ifndef __hemlock_physics_entity_simple_kinematic_entity_h
#define __hemlock_physics_entity_simple_kinematic_entity_h

#include "physics/entity/entity.hpp"

namespace hemlock {
    namespace physics {
        /**
         * @brief Implements a simple bipedal entity. The idea here is that
         * the entity's collisions with the world can be adequately described by a
         * single convex shape like a box or capsule.
         *
         * TODO(Matthew): Implement a CompoundBipod for cases with multiple
         *                shapes. Note here that we should do some research as on
         *                a brief reading, it looks like we would want to attach
         *                the shapes as multiple bodies attached by constraints.
         * TODO(Matthew): Do we need to handle the case of the up direction changing?
         *                Probably not, or at least, for this impl. We might want
         *                a version where it is possible to change direction of gravity
         *                and, in doing, transition up to align with this. 
         */
        class SimpleKinematicEntity : public EntityBase {
        public:
            SimpleKinematicEntity();
            virtual ~SimpleKinematicEntity() { /* Empty */ }

            // TODO(Matthew): revisit...
            void init(btDynamicsWorld* world, const btVector3& dimensions, btScalar mass);
            void init(btDynamicsWorld* world, btScalar radius, btScalar height, btScalar mass);
            void init(btDynamicsWorld* world, btCollisionShape* shape, btScalar mass);

            virtual void set_target_direction(const btQuaternion& direction) override;
            virtual void set_target_velocity(const btVector3& velocity) override;

            void set_default_jump_strength(btScalar default_jump_strength);
            void set_max_step_up(btScalar max_step_up);
            void set_step_speed(btScalar step_speed);

            void step_forward();
            void step_backward();
            void strafe_left();
            void strafe_right();
            void step_direction(const btVector3& direction);

            void jump() { jump(m_default_jump_strength); }
            void jump(btScalar jump_strength);

            void apply_impulse(const btVector3& impulse);

            virtual void updateAction(btCollisionWorld* collision_world, btScalar time) override;

            virtual void debugDraw(btIDebugDraw* drawer) override;
        protected:
            btVector3 m_target_direction, m_target_velocity;
            btScalar  m_default_jump_strength, m_max_step_up, m_step_speed;

            btDynamicsWorld*   m_world;
            btRigidBody*        m_body;
        };
    }
}
namespace hphys = hemlock::physics;

#endif // __hemlock_physics_entity_simple_kinematic_entity_h

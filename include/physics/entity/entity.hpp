#ifndef __hemlock_physics_entity_entity_hpp
#define __hemlock_physics_entity_entity_hpp

#include <bullet/btBulletDynamicsCommon.h>

namespace hemlock {
    namespace physics {
        // TODO(Matthew): Better naming? EntityComponent in ECS is confusing.
        class EntityBase : public btActionInterface {
        public:
            EntityBase() { /* Empty. */ }
            virtual ~EntityBase() { /* Empty. */ }

            virtual void set_target_direction(const btQuaternion& direction) = 0;
            virtual void set_target_velocity(const btVector3& velocity) = 0;
        };

        struct EntityComponent {
            EntityBase* entity;
        };
    }
}
namespace hphys = hemlock::physics;

#endif // __hemlock_physics_entity_entity_hpp

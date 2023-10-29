#ifndef __hemlock_ecs_protected_registry_h
#define __hemlock_ecs_protected_registry_h

namespace hemlock {
    namespace ecs {
        /**
         * @brief Represents what is happening presently to the entity in question. If
         * the entity is dying it should not be processed by another thread, but
         * likewise if any threads are processing it, then it should not be killed.
         */
        enum class EntityState {
            ALIVE      = 0,
            CHANGING   = 1,
            DYING      = 2,
            DEAD       = 3,
            PROCESSING = 4
        };

        /**
         * @brief Control block for entities.
         *
         * TODO(Matthew): right now use EntityState and a counter, could reduce that to
         *                just a counter but could introduce a bit of waiting if there
         *                are many processes coming and going on a single entity.
         *                  actually, as I think about it this will still involve
         *                  waiting but on the less frequent case of the last-process-
         *                  but-this-one having decremented to 0 and set state to ALIVE
         */
        struct EntityControl {
            std::atomic<EntityState> state;
            std::atomic<ui32>        processes;
            std::atomic<bool>        marked_for_death;
        };

        /**
         * @brief Implements necessary synchronisation around a registry to allow for
         * entity lifetime to be controlled by one thread with any threads including
         * that one safely interacting with said entity and its components.
         *
         * NOTE: this does not provide synchronisation around any state of components,
         * for which those components and associated systems are responsible.
         */
        class ProtectedRegistry {
        public:
            // TODO(Matthew): expose all of entt::registry API explicitly or play it
            //                dangerous?
        protected:
            // Underlying ECS registry.
            entt::registry m_registry;

            // Control structure.
            std::unordered_map<entt::entity, EntityControl> m_control_map;
            std::mutex                                      m_control_map_mutex;

            // Used for revisiting entities to die later should they have been under
            // processing.
            std::queue<entt::entity> m_marked_for_death;
        };
    }  // namespace ecs
}  // namespace hemlock
namespace hecs = hemlock::ecs;

#endif  // __hemlock_ecs_protected_registry_h

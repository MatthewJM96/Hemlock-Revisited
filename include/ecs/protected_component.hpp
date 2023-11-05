#ifndef __hemlock_ecs_protected_component_hpp
#define __hemlock_ecs_protected_component_hpp

namespace hemlock {
    namespace ecs {
        // TODO(Matthew): Right now this logic is designed in such a way that a lock is
        //                required per component per entity. Non-trivial but can we
        //                integrate this more closely with EnTT such that we can have a
        //                single lock per thread per entity?

        class ProtectedComponentDeletor {
        public:
            ProtectedComponentDeletor(
                entt::registry* registry,
                std::mutex&     registry_mutex,
                entt::entity    entity
            ) :
                m_registry(registry),
                m_registry_mutex(registry_mutex),
                m_entity(entity) {
                // Empty.
            }

            ~ProtectedComponentDeletor() {
                std::lock_guard lock(m_registry_mutex);

                m_registry->destroy(m_entity);
                m_registry = nullptr;
            }
        protected:
            entt::registry* m_registry;
            std::mutex&     m_registry_mutex;
            entt::entity    m_entity;
        };

        template <typename ComponentData>
        class ProtectedComponent;

        class ProtectedComponentLock {
            template <typename ComponentData>
            friend class ProtectedComponent;
        public:
            ProtectedComponentLock() : m_deletor(nullptr) {
                // Empty.
            }

            ProtectedComponentLock(ProtectedComponentLock&& rhs) :
                m_deletor(std::move(rhs.m_deletor)) {
                // Empty.
            }

            ProtectedComponentLock& operator=(ProtectedComponentLock&& rhs) {
                m_deletor = std::move(rhs.m_deletor);
                return *this;
            }
        protected:
            ProtectedComponentLock(hmem::WeakHandle<ProtectedComponentDeletor> deletor
            ) :
                m_deletor(deletor) {
                // Empty.
            }

            hmem::Handle<ProtectedComponentDeletor> m_deletor;
        };

        template <typename ComponentData>
        class ProtectedComponent {
        public:
            ProtectedComponent(
                hmem::WeakHandle<ProtectedComponentDeletor> deletor, ComponentData data
            ) :
                m_deletor(deletor), m_data(data) {
                // Empty.
            }

            ProtectedComponent(
                hmem::WeakHandle<ProtectedComponentDeletor> deletor,
                ComponentData&&                             data
            ) :
                m_deletor(deletor), m_data(std::forward<ComponentData>(data)) {
                // Empty.
            }

            ComponentData& get(ProtectedComponentLock& lock) {
                lock = ProtectedComponentLock(m_deletor);
                return m_data;
            }
        protected:
            hmem::WeakHandle<ProtectedComponentDeletor> m_deletor;

            ComponentData m_data;
        };
    }  // namespace ecs
}  // namespace hemlock
namespace hecs = hemlock::ecs;

#endif  // __hemlock_ecs_protected_component_hpp

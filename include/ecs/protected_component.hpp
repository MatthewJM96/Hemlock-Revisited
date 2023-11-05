#ifndef __hemlock_ecs_protected_component_hpp
#define __hemlock_ecs_protected_component_hpp

namespace hemlock {
    namespace ecs {
        // TODO(Matthew): Right now this logic is designed in such a way that a lock is
        //                required per component per entity. Non-trivial but can we
        //                integrate this more closely with EnTT such that we can have a
        //                single lock per thread per entity?

        struct ProtectedRegistry {
            ~ProtectedRegistry() {
                // NOTE(Matthew): pretty sure we can guarantee in correct usage no
                //                no thread will ever be accessing this except the one
                //                processing this destructor.

                // Empty.
            }

            entt::registry registry;
            std::mutex     mutex;
        };

        class ProtectedComponentDeletor {
        public:
            ProtectedComponentDeletor() : m_registry(nullptr), m_entity{} {
                // Empty.
            }

            ProtectedComponentDeletor(
                hmem::Handle<ProtectedRegistry> registry, entt::entity entity
            ) :
                m_registry(registry), m_entity(entity) {
                // Empty.
            }

            ~ProtectedComponentDeletor() {
                std::lock_guard lock(m_registry->mutex);

                m_registry->registry.destroy(m_entity);
                m_registry = nullptr;
            }
        protected:
            hmem::Handle<ProtectedRegistry> m_registry;
            entt::entity                    m_entity;
        };

        template <typename ComponentData>
        class ProtectedComponent;

        class ProtectedComponentLock {
        public:
            ProtectedComponentLock() : m_deletor(nullptr) {
                // Empty.
            }

            ProtectedComponentLock(const ProtectedComponentLock& rhs) :
                m_deletor(rhs.m_deletor) {
                // Empty.
            }

            ProtectedComponentLock(ProtectedComponentLock&& rhs) :
                m_deletor(std::move(rhs.m_deletor)) {
                // Empty.
            }

            ProtectedComponentLock(hmem::WeakHandle<ProtectedComponentDeletor> deletor
            ) :
                m_deletor(deletor) {
                // Empty.
            }

            ~ProtectedComponentLock() { m_deletor = nullptr; }

            ProtectedComponentLock& operator=(const ProtectedComponentLock& rhs) {
                m_deletor = rhs.m_deletor;
                return *this;
            }

            ProtectedComponentLock& operator=(ProtectedComponentLock&& rhs) {
                m_deletor = std::move(rhs.m_deletor);
                return *this;
            }

            void release() { m_deletor = nullptr; }
        protected:
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

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

        class ProtectedComponentLock;

        class ProtectedComponentDeletor {
            friend class ProtectedComponentLock;
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

            ~ProtectedComponentLock() { m_deletor = nullptr; }

            ProtectedComponentLock& operator=(const ProtectedComponentLock& rhs) {
                m_deletor = rhs.m_deletor;
                return *this;
            }

            ProtectedComponentLock& operator=(ProtectedComponentLock&& rhs) {
                m_deletor = std::move(rhs.m_deletor);
                return *this;
            }

            bool lock(hmem::WeakHandle<ProtectedComponentDeletor> deletor) {
                // TODO(Matthew): this isn't saving us anything over separate locks per
                //                component so clearly we need to do something else.
                //                probably just allow a get_unsafe that becomes safe so
                //                long as the caller has already got hold of a lock on
                //                the handle.
                if (m_deletor != nullptr) {
                    hmem::Handle<ProtectedComponentDeletor> tmp = deletor.lock();

                    if (tmp == nullptr) return false;

                    return (m_deletor->m_entity == tmp->m_entity)
                           && (m_deletor->m_registry == tmp->m_registry);
                }

                m_deletor = deletor.lock();

                return m_deletor != nullptr;
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

            ComponentData* get(ProtectedComponentLock& lock) {
                if (lock.lock(m_deletor)) {
                    return &m_data;
                }

                return nullptr;
            }
        protected:
            hmem::WeakHandle<ProtectedComponentDeletor> m_deletor;

            ComponentData m_data;
        };
    }  // namespace ecs
}  // namespace hemlock
namespace hecs = hemlock::ecs;

#endif  // __hemlock_ecs_protected_component_hpp

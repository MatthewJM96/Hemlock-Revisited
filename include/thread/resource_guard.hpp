#ifndef __hemlock_thread_resource_guard_hpp
#define __hemlock_thread_resource_guard_hpp

namespace hemlock {
    namespace thread {
        template <typename ResourceType>
        class ResourceGuard {
        public:
            ResourceGuard() : m_resource({}) {
                // Empty.
            }

            ~ResourceGuard() {
                // Empty.
            }

            void init(ResourceType&& resource) {
                std::unique_lock lock(m_mutex);

                m_resource = std::forward<ResourceType>(resource);
            }

            ResourceType& get(std::unique_lock<std::shared_mutex>& lock) {
                lock = std::unique_lock(m_mutex);
                return m_resource;
            }

            const ResourceType& get(std::shared_lock<std::shared_mutex>& lock) {
                lock = std::shared_lock(m_mutex);
                return m_resource;
            }
        protected:
            std::shared_mutex m_mutex;
            ResourceType      m_resource;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_resource_guard_hpp

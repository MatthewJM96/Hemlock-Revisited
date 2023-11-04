#ifndef __hemlock_thread_resource_guard_hpp
#define __hemlock_thread_resource_guard_hpp

namespace hemlock {
    namespace thread {
        template <typename ResourceType>
        class ResourceGuard {
        public:
            using resource_t       = std::decay_t<ResourceType>;
            using const_resource_t = std::conditional_t<
                std::is_pointer_v<std::decay_t<ResourceType>>,
                const std::remove_pointer_t<std::decay_t<ResourceType>>* const,
                const std::decay_t<ResourceType>>;

            ResourceGuard() : m_resource{} {
                // Empty.
            }

            ~ResourceGuard() {
                // Empty.
            }

            void init(resource_t&& resource) {
                std::unique_lock lock(m_mutex);

                m_resource = std::forward<resource_t>(resource);
            }

            resource_t& get(std::unique_lock<std::shared_mutex>& lock) {
                lock = std::unique_lock(m_mutex);
                return m_resource;
            }

            const_resource_t& get(std::shared_lock<std::shared_mutex>& lock) {
                lock = std::shared_lock(m_mutex);
                return m_resource;
            }
        protected:
            std::shared_mutex m_mutex;
            resource_t        m_resource;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_resource_guard_hpp

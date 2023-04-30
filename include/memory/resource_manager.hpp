#ifndef __hemlock_voxel_memory_resource_manager_hpp
#define __hemlock_voxel_memory_resource_manager_hpp

#include "page.hpp"

namespace hemlock {
    namespace memory {
        using SharedResourceLock = std::shared_lock<std::shared_mutex>;
        using UniqueResourceLock = std::unique_lock<std::shared_mutex>;

        template <typename Resource>
        struct PagedResource {
            Page<Resource> data;
            ui32           count;
        };

        template <typename, typename = void, typename = void>
        class ResourceManager;

        template <typename Resource, typename Pager>
        class ResourceManager<
            Resource,
            Pager,
            typename std::enable_if_t<std::is_void_v<Pager>>> {
        public:
            ResourceManager() { /* Empty. */
            }

            ~ResourceManager() { /* Empty. */
            }

            void init();
            void dispose();

            Resource&       get(UniqueResourceLock& lock);
            Resource&       get(UniqueResourceLock& lock, std::defer_lock_t);
            const Resource& get(SharedResourceLock& lock);
            const Resource& get(SharedResourceLock& lock, std::defer_lock_t);
            Resource&       get_unsafe();
        protected:
            std::shared_mutex m_mutex;
            Resource          m_resource;
        };

        template <typename Resource, typename Pager>
        class ResourceManager<
            Resource,
            Pager,
            typename std::enable_if_t<!std::is_void_v<Pager>>> {
            using _PagedResource = PagedResource<Resource>;
        public:
            ResourceManager() : m_resource{ nullptr, 0 } { /* Empty. */
            }

            ~ResourceManager() { /* Empty. */
            }

            void init(hmem::Handle<Pager> pager);
            void dispose();

            _PagedResource&       get(UniqueResourceLock& lock);
            _PagedResource&       get(UniqueResourceLock& lock, std::defer_lock_t);
            const _PagedResource& get(SharedResourceLock& lock);
            const _PagedResource& get(SharedResourceLock& lock, std::defer_lock_t);
            _PagedResource&       get_unsafe();

            virtual void generate_buffer();
            virtual void free_buffer();
        protected:
            hmem::Handle<Pager> m_pager;

            std::shared_mutex m_mutex;
            _PagedResource    m_resource;
        };
    }  // namespace memory
}  // namespace hemlock
namespace hmem = hemlock::memory;

#include "resource_manager.inl"

#endif  // __hemlock_voxel_memory_resource_manager_hpp

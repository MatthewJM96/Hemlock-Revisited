#ifndef __hemlock_memory_paged_allocator_hpp
#define __hemlock_memory_paged_allocator_hpp

#include "handle.hpp"
#include "pager.hpp"

namespace hemlock {
    namespace memory {
        template <typename DataType, size_t PageSize>
        class PagedAllocator : public Allocator<DataType> {
        protected:
            using _Page  = Page<DataType, PageSize>;
            using _Pager = Pager<DataType, PageSize>;
            using _Items = std::vector<DataType*>;
        public:
            PagedAllocator()  { /* Empty. */ }
            ~PagedAllocator() { /* Empty. */ }

            void init(size_t max_free_pages = 3, size_t compaction_factor = 2);
            void dispose();

            Handle<DataType> allocate()                            final;
                        bool deallocate(Handle<DataType>&& handle) final;
        protected:
            bool try_deallocate(Handle<DataType>& handle);

            void do_compaction_if_needed();

            _Pager      m_pager;

            std::mutex  m_free_items_mutex;
            _Items      m_free_items;
        };
    }
}
namespace hmem = hemlock::memory;

#include "paged_allocator.inl"

#endif // __hemlock_memory_paged_allocator_hpp

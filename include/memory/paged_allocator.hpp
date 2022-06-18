#ifndef __hemlock_memory_paged_allocator_hpp
#define __hemlock_memory_paged_allocator_hpp

#include "handle.hpp"
#include "pager.hpp"

namespace hemlock {
    namespace memory {
        template <typename DataType, size_t PageSize>
        requires (PageSize > 0)
        struct PagedAllocatorState {
            ~PagedAllocatorState() {
                std::lock_guard lock(free_items_mutex);
                _Items().swap(free_items);

                pager.dispose();
            }

            using _Page  = Page<DataType, PageSize>;
            using _Pager = Pager<DataType, PageSize>;
            using _Items = std::vector<DataType*>;

            _Pager      pager;
            std::mutex  free_items_mutex;
            _Items      free_items;
        };

        template <typename DataType, size_t PageSize, size_t MaxFreePages>
        requires (PageSize > 0 && MaxFreePages > 0)
        class PagedAllocator {
        public:
            using value_type        = DataType;
            using pointer           = DataType*;
            using const_pointer     = const DataType*;
            using reference         = DataType&;
            using const_reference   = const DataType&;
            using size_type         = std::size_t;
            using difference_type   = std::ptrdiff_t;

            size_type max_size() const;

            template <typename OtherDataType>
            struct rebind {
                using other = PagedAllocator<OtherDataType, PageSize, MaxFreePages>;
            };
        protected:
            using _Page  = typename PagedAllocatorState<DataType, PageSize>::_Page;
            using _Pager = typename PagedAllocatorState<DataType, PageSize>::_Pager;
            using _Items = typename PagedAllocatorState<DataType, PageSize>::_Items;
        public:
            PagedAllocator();
            PagedAllocator(const PagedAllocator<DataType, PageSize, MaxFreePages>& alloc);
            template <typename OtherDataType>
            PagedAllocator(const PagedAllocator<OtherDataType, PageSize, MaxFreePages>& alloc);
            ~PagedAllocator() { /* Empty. */ }

            pointer allocate(size_type count, const void* = 0);

            template <typename ...Args>
            void construct(pointer data, Args&&... args);

            void deallocate(pointer data, size_type count);
        protected:
            std::shared_ptr<PagedAllocatorState<DataType, PageSize>> m_state;
        };
    }
}
namespace hmem = hemlock::memory;

#include "paged_allocator.inl"

#endif // __hemlock_memory_paged_allocator_hpp

#ifndef __hemlock_memory_paged_allocator_hpp
#define __hemlock_memory_paged_allocator_hpp

#include "handle.hpp"
#include "heterogenous_pager.hpp"

namespace hemlock {
    namespace memory {
        template <size_t PageSize, size_t MaxFreePages>
            requires (PageSize > 0)
        struct PagedAllocatorState {
            ~PagedAllocatorState() {
                std::lock_guard<std::mutex> lock(free_items_mutex);
                _Items().swap(free_items);

                pager.dispose();
            }

            using _Pager = HeterogenousPager<PageSize, MaxFreePages>;
            using _Items = std::unordered_map<size_t, std::vector<void*>>;

            _Pager     pager;
            std::mutex free_items_mutex;
            _Items     free_items;
        };

        template <typename DataType, size_t PageSize, size_t MaxFreePages>
            requires (PageSize > 0 && MaxFreePages > 0)
        class PagedAllocator {
            template <typename D, size_t P, size_t M>
                requires (P > 0 && M > 0)
            friend class PagedAllocator;
        public:
            using value_type      = DataType;
            using pointer         = DataType*;
            using const_pointer   = const DataType*;
            using reference       = DataType&;
            using const_reference = const DataType&;
            using size_type       = std::size_t;
            using difference_type = std::ptrdiff_t;

            size_type max_size() const;

            template <typename OtherDataType>
            struct rebind {
                using other = PagedAllocator<OtherDataType, PageSize, MaxFreePages>;
            };
        protected:
            using _Page  = Page<DataType>;
            using _Pager = typename PagedAllocatorState<PageSize, MaxFreePages>::_Pager;
            using _Items = typename PagedAllocatorState<PageSize, MaxFreePages>::_Items;
        public:
            PagedAllocator();
            PagedAllocator(const PagedAllocator<DataType, PageSize, MaxFreePages>& alloc
            );
            template <typename OtherDataType>
            PagedAllocator(
                const PagedAllocator<OtherDataType, PageSize, MaxFreePages>& alloc
            );

            ~PagedAllocator() { /* Empty. */
            }

            pointer allocate(size_type count, const void* = 0);

            template <typename... Args>
            void construct(pointer data, Args&&... args);

            void deallocate(pointer data, size_type count);
        protected:
            Handle<PagedAllocatorState<PageSize, MaxFreePages>> m_state;
        };
    }  // namespace memory
}  // namespace hemlock
namespace hmem = hemlock::memory;

#include "paged_allocator.inl"

#endif  // __hemlock_memory_paged_allocator_hpp

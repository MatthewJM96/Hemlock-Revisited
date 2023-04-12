#ifndef __hemlock_memory_paged_allocator_hpp
#define __hemlock_memory_paged_allocator_hpp

#include "handle.hpp"
#include "heterogenous_pager.hpp"

namespace hemlock {
    namespace memory {
        struct PageExtent {
            ptrdiff_t begin, end;
        };

        struct PageTracker {
            std::vector<void*>                         allocated_pages;
            std::unordered_multimap<void*, PageExtent> free_extents;
        };

        template <size_t PageSize, size_t MaxFreePages>
            requires (PageSize > 0)
        struct PagedAllocatorState {
            using _Pager       = HeterogenousPager<PageSize, MaxFreePages>;
            using _PageTracker = std::unordered_map<size_t, PageTracker>;

            PagedAllocatorState() { /* Empty. */
            }

            ~PagedAllocatorState() {
                std::lock_guard<std::mutex> lock(page_tracker_mutex);
                _PageTracker().swap(page_tracker);

                // TODO(Matthew): How are we ensuring paged memory is fully deallocated?
                //                Need to track pages seperately from items, and items
                //                can be tracked in membership via pointer arithmetic or
                //                however works based on if and for what we need to
                //                track that (e.g. for compaction).

                pager.dispose();
            }

            _Pager       pager;
            std::mutex   page_tracker_mutex;
            _PageTracker page_tracker;
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
            using _PageTracker =
                typename PagedAllocatorState<PageSize, MaxFreePages>::_PageTracker;
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

            size_t allocated_bytes() { return m_state->pager.allocated_bytes(); }

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

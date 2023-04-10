#ifndef __hemlock_memory_heterogenous_pager_hpp
#define __hemlock_memory_heterogenous_pager_hpp

#include "pager.hpp"

namespace hemlock {
    namespace memory {
        template <size_t MaxFreePages>
            requires (MaxFreePages > 0)
        struct HeterogenousPageInfo {
            size_t free_page_count;
            size_t total_page_count;
            size_t data_byte_size;

            Pages<void, MaxFreePages> pages;
        };
        template <size_t MaxFreePages>
            requires (MaxFreePages > 0)
        using HeterogenousPageInfos = std::unordered_map<size_t, HeterogenousPageInfo<MaxFreePages>>;

        template <size_t PageSize, size_t MaxFreePages>
            requires (PageSize > 0 && MaxFreePages > 0)
        class HeterogenousPager {
        protected:
            using _Pages                 = Pages<void, MaxFreePages>;
            using _HeterogenousPageInfos = HeterogenousPageInfos<MaxFreePages>;
        public:
            HeterogenousPager() { /* Empty. */
            }

            ~HeterogenousPager() { /* Empty. */
            }

            /**
             * @brief Dispose of the pager, note that this does not handle any pages
             * that have been handed out to anyone, so calling this implies you have
             * correctly disposed of any callers of get_page (and their associated
             * pages).
             */
            void dispose();

            /**
             * @brief Provides the number of allocated bytes of this pager. Unlike
             * Pager::allocated_bytes this method is thread-safe and provides an
             * atomically accurate calculation of allocated bytes.
             */
            size_t allocated_bytes();

            /**
             * @brief Returns a page to the caller. The page's lifetime is controlled by
             * the caller, and it must be freed via a call to free_page from this same
             * pager.
             *
             * @tparam DataType The type of data held by the page.
             */
            template <typename DataType>
            Page<DataType> get_page();
            /**
             * @brief Frees the passed-in page.
             *
             * @tparam DataType The type of data held by the page.
             *
             * @param page The page to free.
             */
            template <typename DataType>
            void free_page(Page<DataType> page);
        protected:
            std::mutex             m_heterogenous_pages_mutex;
            _HeterogenousPageInfos m_heterogenous_page_infos;
        };
    }  // namespace memory
}  // namespace hemlock
namespace hmem = hemlock::memory;

#include "heterogenous_pager.inl"

#endif  // __hemlock_memory_heterogenous_pager_hpp

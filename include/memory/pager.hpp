#ifndef __hemlock_memory_pager_hpp
#define __hemlock_memory_pager_hpp

#include "page.hpp"

namespace hemlock {
    namespace memory {
        template <typename DataType, size_t PageSize, size_t MaxFreePages>
            requires (PageSize > 0 && MaxFreePages > 0)
        class Pager {
        protected:
            using _Page  = Page<DataType>;
            using _Pages = Pages<DataType, MaxFreePages>;
        public:
            Pager() : m_free_page_count(0), m_total_page_count(0) { /* Empty. */
            }

            ~Pager() { /* Empty. */
            }

            /**
             * @brief Dispose of the pager, note that this does not handle any pages
             * that have been handed out to anyone, so calling this implies you have
             * correctly disposed of any callers of get_page (and their associated
             * pages).
             */
            void dispose();

            /**
             * @brief Provides the number of allocated bytes of this pager. Note that
             * this does not do this in a thread-safe manner, so it is not necessarily
             * atomically accurate.
             */
            size_t allocated_bytes();

            /**
             * @brief Returns a page to the caller. The page's lifetime is controlled by
             * the caller, and it must be freed via a call to free_page from this same
             * pager.
             */
            _Page get_page();
            /**
             * @brief Frees the passed-in page.
             *
             * @param page The page to free.
             */
            void free_page(_Page page);
        protected:
            std::mutex m_free_pages_mutex;
            _Pages     m_free_pages;
            size_t     m_free_page_count;
            size_t     m_total_page_count;
        };
    }  // namespace memory
}  // namespace hemlock
namespace hmem = hemlock::memory;

#include "pager.inl"

#endif  // __hemlock_memory_pager_hpp

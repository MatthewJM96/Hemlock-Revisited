#ifndef __hemlock_memory_pager_hpp
#define __hemlock_memory_pager_hpp

// TODO(Matthew): We don't currently track issued but not free pages.
//                This means a memory leak can occur if pager is
//                disposed without first explicitly freeing all pages.

namespace hemlock {
    namespace memory {
        template <typename DataType>
        using Page = DataType*;

        template <typename DataType, size_t MaxFreePages>
            requires (MaxFreePages > 0)
        using Pages = std::array<Page<DataType>, MaxFreePages>;

        template <typename DataType, size_t PageSize, size_t MaxFreePages>
            requires (PageSize > 0 && MaxFreePages > 0)
        class Pager {
        protected:
            using _Page  = Page<DataType>;
            using _Pages = Pages<DataType, MaxFreePages>;
        public:
            Pager() : m_free_page_count(0) { /* Empty. */
            }

            ~Pager() { /* Empty. */
            }

            void dispose();

            _Page get_page();
            void  free_page(_Page page);
        protected:
            std::mutex m_free_pages_mutex;
            _Pages     m_free_pages;
            size_t     m_free_page_count;
        };
    }  // namespace memory
}  // namespace hemlock
namespace hmem = hemlock::memory;

#include "pager.inl"

#endif  // __hemlock_memory_pager_hpp

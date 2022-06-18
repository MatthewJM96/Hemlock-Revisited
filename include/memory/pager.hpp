#ifndef __hemlock_memory_pager_hpp
#define __hemlock_memory_pager_hpp

namespace hemlock {
    namespace memory {
        template <typename DataType, size_t PageSize>
        requires (PageSize > 0)
        using Page = std::span<DataType, PageSize>;

        template <typename DataType, size_t PageSize>
        requires (PageSize > 0)
        using Pages = std::vector<Page<DataType, PageSize>>;

        template <typename DataType, size_t PageSize>
        requires (PageSize > 0)
        class Pager {
        protected:
            using _Page  = Page<DataType, PageSize>;
            using _Pages = Pages<DataType, PageSize>;
        public:
            Pager()  { /* Empty. */ }
            ~Pager() { /* Empty. */ }

            void init(size_t max_free_pages);
            void dispose();

            _Page get_page();
            void free_page(_Page page);
        protected:
            std::mutex  m_free_pages_mutex;
            _Pages      m_free_pages;
        };
    }
}
namespace hmem = hemlock::memory;

#include "pager.inl"

#endif // __hemlock_memory_pager_hpp

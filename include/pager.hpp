#ifndef __hemlock_pager_hpp
#define __hemlock_pager_hpp

namespace hemlock {
    template <typename DataType, size_t PageSize>
    using Page = std::span<DataType, PageSize>;

    template <typename DataType, size_t PageSize>
    using Pages = std::vector<Page<DataType, PageSize>>;

    template <typename DataType, size_t PageSize>
    requires (PageSize > 0)
    class Pager {
    protected:
        using M_Page  = Page<DataType, PageSize>;
        using M_Pages = Pages<DataType, PageSize>;
    public:
        Pager();
        ~Pager() { /* Empty. */ }

        void init(size_t max_free_pages);
        void dispose();

        M_Page get_page();
        void free_page(M_Page page);
    protected:
        std::mutex  m_free_pages_mutex;

        M_Pages     m_free_pages;
    };
}

#include "pager.inl"

#endif // __hemlock_pager_hpp

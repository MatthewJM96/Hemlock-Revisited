#ifndef __hemlock_pager_hpp
#define __hemlock_pager_hpp

namespace hemlock {
    template <typename DataType, ui32 MaxFreePages>
    using PageList = std::array<DataType*, MaxFreePages>;

    template <typename DataType, size_t PageSize, ui32 MaxFreePages>
    class ThreadSafePager {
        using _PageList = PageList<DataType, MaxFreePages>;
    public:
        ThreadSafePager();
        ThreadSafePager(const ThreadSafePager&) = delete;
        ThreadSafePager(ThreadSafePager&& rhs);

        DataType* get_page();

        void free_page(DataType* page);
    protected:
        std::mutex  m_lock;
        ui32        m_number_free_pages;
        _PageList   m_free_pages;
    };
}

#include "pager.inl"

#endif // __hemlock_pager_hpp

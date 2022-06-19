template <typename DataType, size_t PageSize, ui32 MaxFreePages>
hemlock::ThreadSafePager<DataType, PageSize, MaxFreePages>::ThreadSafePager() :
    m_number_free_pages(0)
{
    // Empty.
}

template <typename DataType, size_t PageSize, ui32 MaxFreePages>
hemlock::ThreadSafePager<DataType, PageSize, MaxFreePages>::ThreadSafePager(ThreadSafePager&& rhs)
{
    std::lock_guard<std::mutex> lock(rhs.m_lock);

    m_number_free_pages = rhs.m_number_free_pages;
    m_free_pages        = std::move(rhs.m_free_pages);
}

template <typename DataType, size_t PageSize, ui32 MaxFreePages>
DataType* hemlock::ThreadSafePager<DataType, PageSize, MaxFreePages>::get_page() {
    std::lock_guard<std::mutex> lock(m_lock);

    if (m_number_free_pages == 0)
        return new DataType[PageSize];

    return m_free_pages[--m_number_free_pages];
}

template <typename DataType, size_t PageSize, ui32 MaxFreePages>
void hemlock::ThreadSafePager<DataType, PageSize, MaxFreePages>::free_page(DataType* page) {
    std::lock_guard<std::mutex> lock(m_lock);

    if (m_number_free_pages == MaxFreePages) {
        delete[] page;
    } else {
        m_free_pages[m_number_free_pages++] = page;
    }
}

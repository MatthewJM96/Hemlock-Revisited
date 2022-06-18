template <typename DataType, size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
void hmem::Pager<DataType, PageSize, MaxFreePages>::dispose() {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

    for (auto& free_page : m_free_pages) {
        delete[] free_page;
    }

    _Pages().swap(m_free_pages);
    m_free_page_count = 0;
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
hmem::Page<DataType> hmem::Pager<DataType, PageSize, MaxFreePages>::get_page() {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

    if (m_free_page_count > 0)
        return m_free_pages[--m_free_page_count];

    return reinterpret_cast<_Page>(new ui8[sizeof(DataType) * PageSize]);
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
void hmem::Pager<DataType, PageSize, MaxFreePages>::free_page(Page<DataType> page) {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);
    
    if (m_free_page_count < MaxFreePages) {
        m_free_pages[m_free_page_count] = page;
    } else {
        delete[] page;
    }
}

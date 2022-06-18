template <size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
void hmem::HeterogenousPager<PageSize, MaxFreePages>::dispose() {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

    for (auto& [type, pages] : m_free_pages) {
        for (auto& page : pages) {
            delete[] page;
        }
    }

    _HeterogenousPages().swap(m_free_pages);

    for (auto& [type, count] : m_free_page_counts) {
        count = 0;
    }
}

template <size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
template <typename DataType>
hmem::Page<DataType> hmem::HeterogenousPager<PageSize, MaxFreePages>::get_page() {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

    auto page_type = typeid(DataType).hash_code();

    m_free_pages.try_emplace(page_type, Pages<void, MaxFreePages>{});
    m_free_page_counts.try_emplace(page_type, 0);

    if (m_free_page_counts[page_type] > 0)
        return reinterpret_cast<Page<DataType>>(m_free_pages[page_type][--m_free_page_counts[page_type]]);

    return reinterpret_cast<Page<DataType>>(new ui8[sizeof(DataType) * PageSize]);
}

template <size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
template <typename DataType>
void hmem::HeterogenousPager<PageSize, MaxFreePages>::free_page(Page<DataType> page) {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

    auto page_type = typeid(DataType).hash_code();

    m_free_pages.try_emplace(page_type, Pages<void, MaxFreePages>{});
    m_free_page_counts.try_emplace(page_type, 0);

    if (m_free_page_counts[page_type] < MaxFreePages) {
        m_free_pages[page_type][m_free_page_counts[page_type]++] = page;
    } else {
        delete[] page;
    }
}

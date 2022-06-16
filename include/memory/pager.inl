template <typename DataType, size_t PageSize>
void hmem::Pager<DataType, PageSize>::init(size_t max_free_pages) {
    // NOTE: We don't resize if m_free_pages already has non-zero size/capacity.
    //          Assuming init/dispose called correctly.
    m_free_pages.reserve(max_free_pages);
}

template <typename DataType, size_t PageSize>
void hmem::Pager<DataType, PageSize>::dispose() {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

    for (auto& free_page : m_free_pages) {
        delete[] free_page.data();
    }

    _Pages().swap(m_free_pages);
}

template <typename DataType, size_t PageSize>
hmem::Page<DataType, PageSize> hmem::Pager<DataType, PageSize>::get_page() {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

    if (m_free_pages.size() > 0) {
        _Page page = m_free_pages.back();
        m_free_pages.pop_back();
        return page;
    }

    return { new DataType[PageSize] };
}

template <typename DataType, size_t PageSize>
void hmem::Pager<DataType, PageSize>::free_page(Page<DataType, PageSize> page) {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);
    
    if (m_free_pages.size() < m_free_pages.capacity()) {
        m_free_pages.emplace_back(page);
    } else {
        delete[] page.data();
    }
}

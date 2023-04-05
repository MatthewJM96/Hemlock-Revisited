template <size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
void hmem::HeterogenousPager<PageSize, MaxFreePages>::dispose() {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

#if DEBUG
    for (auto& [type, metadata] : m_page_metadata) {
        assert(metadata.free_pages == metadata.total_pages);
    }
#endif

    for (auto& [type, pages] : m_free_pages) {
        for (auto& page : pages) {
            delete[] page;
        }
    }

    for (auto& [type, metadata] : m_page_metadata) {
        metadata.free_pages = 0;
        metadata.total_pages = 0;
    }

    _HeterogenousPages().swap(m_free_pages);
}

template <size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
size_t hmem::HeterogenousPager<PageSize, MaxFreePages>::allocated_bytes() {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

    size_t allocated_bytes = 0;

    for (auto& [type, metadata] : m_page_metadata) {
        allocated_bytes += metadata.total_pages * metadata.data_byte_size * PageSize;
    }
}

template <size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
template <typename DataType>
hmem::Page<DataType> hmem::HeterogenousPager<PageSize, MaxFreePages>::get_page() {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

    size_t page_type = typeid(DataType).hash_code();

    m_free_pages.try_emplace(page_type, Pages<void, MaxFreePages>{});
    m_page_metadata.try_emplace(page_type, {0,0,sizeof(DataType)});

    if (m_page_metadata[page_type].free_pages > 0)
        return reinterpret_cast<Page<DataType>>(
            m_free_pages[page_type][--m_page_metadata[page_type].free_pages]
        );

    ++m_page_metadata[page_type].total_pages;

    return reinterpret_cast<Page<DataType>>(new ui8[sizeof(DataType) * PageSize]);
}

template <size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
template <typename DataType>
void hmem::HeterogenousPager<PageSize, MaxFreePages>::free_page(Page<DataType> page) {
    std::lock_guard<std::mutex> lock(m_free_pages_mutex);

    size_t page_type = typeid(DataType).hash_code();

    if (m_free_page_counts[page_type] < MaxFreePages) {
        m_free_pages[page_type][m_page_metadata[page_type].free_pages++] = page;
    } else {
        --m_page_metadata[page_type].total_pages;

        delete[] page;
    }
}

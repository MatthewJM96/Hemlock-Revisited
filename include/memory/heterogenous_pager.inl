template <size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
void hmem::HeterogenousPager<PageSize, MaxFreePages>::dispose() {
    std::lock_guard<std::mutex> lock(m_heterogenous_pages_mutex);

#if DEBUG
    // TODO(Matthew): Until we actually dispose of pages correctly this is just gonna
    // fail always. for (auto& [type, heterogenous_page_info] :
    // m_heterogenous_page_infos) {
    //     assert(heterogenous_page_info.free_page_count ==
    //     heterogenous_page_info.total_page_count);
    // }
#endif

    for (auto& [type, heterogenous_page_info] : m_heterogenous_page_infos) {
        for (auto& page : heterogenous_page_info.pages) {
            delete[] reinterpret_cast<ui8*>(page);
        }

        heterogenous_page_info.free_page_count  = 0;
        heterogenous_page_info.total_page_count = 0;
    }

    _HeterogenousPageInfos().swap(m_heterogenous_page_infos);
}

template <size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
size_t hmem::HeterogenousPager<PageSize, MaxFreePages>::allocated_bytes() {
    std::lock_guard<std::mutex> lock(m_heterogenous_pages_mutex);

    size_t allocated_bytes = 0;

    for (auto& [type, heterogenous_page_info] : m_heterogenous_page_infos) {
        allocated_bytes += heterogenous_page_info.total_page_count
                           * heterogenous_page_info.data_byte_size * PageSize;
    }

    return allocated_bytes;
}

template <size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
template <typename DataType>
hmem::Page<DataType> hmem::HeterogenousPager<PageSize, MaxFreePages>::get_page() {
    std::lock_guard<std::mutex> lock(m_heterogenous_pages_mutex);

    size_t page_type = typeid(DataType).hash_code();

    m_heterogenous_page_infos.try_emplace(
        page_type,
        HeterogenousPageInfo<MaxFreePages>{ 0, 0, sizeof(DataType), _Pages{} }
    );

    auto& page_info = m_heterogenous_page_infos[page_type];

    if (page_info.free_page_count > 0)
        return reinterpret_cast<Page<DataType>>(
            page_info.pages[--page_info.free_page_count]
        );

    ++page_info.total_page_count;

    return reinterpret_cast<Page<DataType>>(new ui8[sizeof(DataType) * PageSize]);
}

template <size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
template <typename DataType>
void hmem::HeterogenousPager<PageSize, MaxFreePages>::free_page(Page<DataType> page) {
    std::lock_guard<std::mutex> lock(m_heterogenous_pages_mutex);

    size_t page_type = typeid(DataType).hash_code();

    m_heterogenous_page_infos.try_emplace(
        page_type,
        HeterogenousPageInfo<MaxFreePages>{ 0, 0, sizeof(DataType), _Pages{} }
    );

    auto& page_info = m_heterogenous_page_infos[page_type];

    if (page_info.free_page_count < MaxFreePages) {
        page_info.pages[page_info.free_page_count++] = page;
    } else {
        --page_info.total_page_count;

        delete[] reinterpret_cast<ui8*>(page);
    }
}

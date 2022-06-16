
template <hmem::IsHandleable DataType, size_t PageSize>
void hmem::PagedAllocator<DataType, PageSize>::init(size_t max_free_pages /*= 3*/, size_t compaction_factor /*= 2*/) {
    std::lock_guard<std::mutex> lock(m_free_items_mutex);

    m_pager.init(max_free_pages);

    // NOTE: We don't resize if m_free_items already has non-zero size/capacity.
    //          Assuming init/dispose called correctly.
    m_free_items.reserve(PageSize * compaction_factor);
}

template <hmem::IsHandleable DataType, size_t PageSize>
void hmem::PagedAllocator<DataType, PageSize>::dispose() {
    std::lock_guard<std::mutex> lock(m_free_items_mutex);

    m_pager.dispose();

    _Items().swap(m_free_items);
}

template <hmem::IsHandleable DataType, size_t PageSize>
hmem::Handle<DataType> hmem::PagedAllocator<DataType, PageSize>::allocate() {
    std::lock_guard<std::mutex> lock(m_free_items_mutex);

    if (m_free_items.size() > 0) {
        DataType item = m_free_items.back();
        m_free_items.pop_back();
        return Handle<DataType>(item, this);
    }

    _Page page = m_pager.get_page();
    // Can we use a range transform here? This is a bit ugly.
    auto page_ptrs = std::ranges::transform_view(page, [](DataType& el) { return &el; });
    m_free_items.insert(m_free_items.end(), std::begin(page_ptrs) + 1, std::end(page_ptrs));

    return Handle<DataType>(&page[0], this);
}

template <hmem::IsHandleable DataType, size_t PageSize>
bool hmem::PagedAllocator<DataType, PageSize>::deallocate(Handle<DataType>&& handle) {
    std::lock_guard<std::mutex> lock(m_free_items_mutex);

    if (handle == nullptr) return false;

    m_free_items.emplace_back(handle.m_data);

    // Need to think about how to even do this, given live handles can hardly be notified of this.
    do_compaction_if_needed();

    return true;
}

template <hmem::IsHandleable DataType, size_t PageSize>
void hmem::PagedAllocator<DataType, PageSize>::do_compaction_if_needed() {
    // Empty... for now.
}
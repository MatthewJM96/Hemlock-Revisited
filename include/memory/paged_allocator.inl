template <typename DataType, size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
typename hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::size_type
    hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::max_size() const
{
    return PageSize;
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::PagedAllocator() {
    m_state = std::make_shared<PagedAllocatorState<DataType, PageSize, MaxFreePages>>();
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::PagedAllocator(
    const PagedAllocator<DataType, PageSize, MaxFreePages>& alloc
) {
    m_state = alloc.m_state;
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
template <typename OtherDataType>
hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::PagedAllocator(
    const PagedAllocator<OtherDataType, PageSize, MaxFreePages>& alloc
) {
    // TODO(Matthew): this patently isn't going to work. we need to somehow
    //                make the pager more generic so that it can allocate pages
    //                with type information at call to get_page.
    // m_state = alloc.m_state;
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
typename hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::pointer
    hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::allocate(size_type count, const void* /*= 0*/)
{
    if (count == 0) return nullptr;

    std::lock_guard<std::mutex> lock(m_state->free_items_mutex);

    if (m_state->free_items.size() > 0) {
        pointer item = m_state->free_items.back();
        m_state->free_items.pop_back();
        return item;
    }

    _Page page = m_state->pager.get_page();

    // Add all items in page after `count` first items
    // to the free items list.
    if (count < PageSize) {
        // auto item_ptrs = std::ranges::transform_view(page, [](DataType& el) { return &el; });
        // m_state->free_items.insert(m_state->free_items.end(), std::begin(item_ptrs) + count, std::end(item_ptrs));

        for (size_t i = count; i < PageSize; ++i) m_state->free_items.emplace_back(&page[i]);
    }

    return &page[0];
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
template <typename ...Args>
void hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::construct(pointer data, Args&&... args) {
    new ((void*)data) DataType(args...);
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
requires (PageSize > 0 && MaxFreePages > 0)
void hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::deallocate(pointer data, size_type count) {
    if (data == nullptr || count == 0) return;

    std::lock_guard<std::mutex> lock(m_state->free_items_mutex);

    // TODO(Matthew): is this notably slower for the case of count == 1 than emplace_back?
    // auto items = std::span(data, count);
    // auto item_ptrs = std::ranges::transform_view(items, [](DataType& el) { return &el; });
    // m_state->free_items.insert(m_state->free_items.end(), std::begin(item_ptrs), std::end(item_ptrs));

    for (size_t i = 0; i < count; ++i) m_state->free_items.emplace_back(&data[i]);

    // Need to think about how to even do this, given live handles can hardly be notified of this.
    //   Probably can compact only by being more careful about which free item is next used as
    //   an allocation - perhaps keep each page's free items in a separate bucket and prefer
    //   to allocate to the most full bucket with availability?
    //do_compaction_if_needed();
}

// template <typename DataType, size_t PageSize, size_t MaxFreePages>
// void hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::do_compaction_if_needed() {
//     // Empty... for now.
// }
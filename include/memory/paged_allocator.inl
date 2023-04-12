template <typename DataType, size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
typename hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::size_type
    hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::max_size() const {
    return PageSize;
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::PagedAllocator() {
    m_state = make_handle<PagedAllocatorState<PageSize, MaxFreePages>>();
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
    m_state = alloc.m_state;
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
typename hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::pointer
    hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::
        allocate(size_type count, const void* /*= 0*/) {
    if (count == 0) return nullptr;

#if DEBUG
    assert(count <= PageSize);
#endif

    std::lock_guard<std::mutex> lock(m_state->page_tracker_mutex);

    size_t page_type = typeid(DataType).hash_code();

    // TODO(Matthew): std vector likely has a resizing pattern unsuitable for us, should
    //                profile this versus a custom version.
    //                  regardless can certainly expose this as a template param
    m_state->page_tracker.try_emplace(page_type, PageTracker{});

    auto& page_tracker = m_state->page_tracker[page_type];

    // Determine if any extent currently exists sufficiently large to provide the
    // allocation, and provide it if so - appropriately updating the extent.

    for (auto extent_it = page_tracker.free_extents.begin();
         extent_it != page_tracker.free_extents.end();
         ++extent_it)
    {
        auto& [page, extent] = *extent_it;

        if (extent.end - extent.begin > count) {
            pointer item = reinterpret_cast<pointer>(page) + extent.begin;

            extent.begin += count;

            return item;
        } else if (extent.end - extent.begin == count) {
            pointer item = reinterpret_cast<pointer>(page) + extent.begin;

            page_tracker.free_extents.erase(extent_it);

            return item;
        }
    }

    // If no extent was available, then get a new page and provide an extent from that.

    _Page page = m_state->pager.template get_page<DataType>();

    page_tracker.allocated_pages.emplace_back(page);

    if (count < PageSize) {
        page_tracker.free_extents.emplace(
            std::make_pair<void*, PageExtent>(page, PageExtent{ count, PageSize })
        );
    }

    return page;
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
template <typename... Args>
void hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::construct(
    pointer data, Args&&... args
) {
    new ((void*)data) DataType(std::forward<Args>(args)...);
}

template <typename DataType, size_t PageSize, size_t MaxFreePages>
    requires (PageSize > 0 && MaxFreePages > 0)
void hmem::PagedAllocator<DataType, PageSize, MaxFreePages>::deallocate(
    pointer data, size_type count
) {
    if (data == nullptr || count == 0) return;

    std::lock_guard<std::mutex> lock(m_state->page_tracker_mutex);

    auto page_type = typeid(DataType).hash_code();

    m_state->page_tracker.try_emplace(page_type, PageTracker{});

    auto& page_tracker = m_state->page_tracker[page_type];

    // Find the page to which this data belongs.

    void* page_void = nullptr;
    _Page page      = nullptr;
    for (void* page_void_tmp : page_tracker.allocated_pages) {
        _Page page_tmp = reinterpret_cast<_Page>(page_void_tmp);
        if (data >= page_tmp && data <= page_tmp + PageSize) {
#if DEBUG
            assert(data - page_tmp + count <= PageSize);
#endif
            page      = page_tmp;
            page_void = page_void_tmp;

            break;
        }
    }

#if DEBUG
    assert(page != nullptr);
#else
    if (page == nullptr) return;
#endif

    // For all extents currently free in page, attach this deallocated extent to those
    // it is contiguous with, erasing any second extent if it borders two of them.

    PageExtent* attached_to_extent = nullptr;
    auto        extents            = page_tracker.free_extents.equal_range(page_void);
    for (auto extent_it = extents.first; extent_it != extents.second; ++extent_it) {
        auto& [_, extent] = *extent_it;

        if (data - page + count == extent.begin) {
#if DEBUG
            assert(count <= extent.begin);
#endif

            if (attached_to_extent) {
                attached_to_extent->end = extent.end;
                page_tracker.free_extents.erase(extent_it);
                break;
            } else {
                extent.begin       -= count;
                attached_to_extent = &extent;
            }
        } else if (data - page == extent.end) {
            if (attached_to_extent) {
                attached_to_extent->begin = extent.begin;
                page_tracker.free_extents.erase(extent_it);
                break;
            } else {
                extent.end         += count;
                attached_to_extent = &extent;
            }
        }
    }

    // If no extent was attached to, then we need to add an extent.
    if (!attached_to_extent)
        page_tracker.free_extents.emplace(std::make_pair<void*, PageExtent>(
            std::move(page_void), PageExtent{ data - page, data - page + count }
        ));

    // Need to think about how to even do this, given live handles can hardly be
    // notified of this.
    //   Probably can compact only by being more careful about which free item is next
    //   used as an allocation - perhaps keep each page's free items in a separate
    //   bucket and prefer to allocate to the most full bucket with availability?
    // do_compaction_if_needed();
}

// template <typename DataType, size_t PageSize, size_t MaxFreePages>
// void hmem::PagedAllocator<DataType, PageSize,
// MaxFreePages>::do_compaction_if_needed() {
//     // Empty... for now.
// }

template <typename Resource, typename Pager>
void hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<std::is_void_v<Pager>>>::init() {
    // Empty.
}

template <typename Resource, typename Pager>
void hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<std::is_void_v<Pager>>>::dispose() {
    this->free_buffer();
}

template <typename Resource, typename Pager>
Resource& hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<std::is_void_v<Pager>>>::get(UniqueResourceLock& lock) {
    lock = UniqueResourceLock(m_mutex);
    return m_resource;
}

template <typename Resource, typename Pager>
Resource& hmem::
    ResourceManager<Resource, Pager, typename std::enable_if_t<std::is_void_v<Pager>>>::
        get(UniqueResourceLock& lock, std::defer_lock_t) {
    lock = UniqueResourceLock(m_mutex, std::defer_lock);
    return m_resource;
}

template <typename Resource, typename Pager>
const Resource& hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<std::is_void_v<Pager>>>::get(SharedResourceLock& lock) {
    lock = SharedResourceLock(m_mutex);
    return m_resource;
}

template <typename Resource, typename Pager>
const Resource& hmem::
    ResourceManager<Resource, Pager, typename std::enable_if_t<std::is_void_v<Pager>>>::
        get(SharedResourceLock& lock, std::defer_lock_t) {
    lock = SharedResourceLock(m_mutex, std::defer_lock);
    return m_resource;
}

template <typename Resource, typename Pager>
Resource& hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<std::is_void_v<Pager>>>::get_unsafe() {
    return m_resource;
}

template <typename Resource, typename Pager>
void hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<!std::is_void_v<Pager>>>::init(hmem::Handle<Pager> pager
) {
    m_pager = pager;
}

template <typename Resource, typename Pager>
void hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<!std::is_void_v<Pager>>>::dispose() {
    this->free_buffer();

    m_pager = nullptr;
}

template <typename Resource, typename Pager>
hmem::PagedResource<Resource>& hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<!std::is_void_v<Pager>>>::get(UniqueResourceLock& lock) {
    lock = UniqueResourceLock(m_mutex);
    return m_resource;
}

template <typename Resource, typename Pager>
hmem::PagedResource<Resource>& hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<!std::is_void_v<Pager>>>::
    get(UniqueResourceLock& lock, std::defer_lock_t) {
    lock = UniqueResourceLock(m_mutex, std::defer_lock);
    return m_resource;
}

template <typename Resource, typename Pager>
const hmem::PagedResource<Resource>& hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<!std::is_void_v<Pager>>>::get(SharedResourceLock& lock) {
    lock = SharedResourceLock(m_mutex);
    return m_resource;
}

template <typename Resource, typename Pager>
const hmem::PagedResource<Resource>& hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<!std::is_void_v<Pager>>>::
    get(SharedResourceLock& lock, std::defer_lock_t) {
    lock = SharedResourceLock(m_mutex, std::defer_lock);
    return m_resource;
}

template <typename Resource, typename Pager>
hmem::PagedResource<Resource>& hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<!std::is_void_v<Pager>>>::get_unsafe() {
    return m_resource;
}

template <typename Resource, typename Pager>
void hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<!std::is_void_v<Pager>>>::generate_buffer() {
    UniqueResourceLock lock(m_mutex);

    m_resource.count = 0;
    if (!m_resource.data) m_resource.data = m_pager->get_page();
}

template <typename Resource, typename Pager>
void hmem::ResourceManager<
    Resource,
    Pager,
    typename std::enable_if_t<!std::is_void_v<Pager>>>::free_buffer() {
    UniqueResourceLock lock(m_mutex);

    m_resource.count = 0;
    if (m_resource.data) m_pager->free_page(m_resource.data);
    m_resource.data = nullptr;
}

template <hmem::Handleable UnderlyingType>
hmem::Handle<UnderlyingType>::Handle() :
    m_data(nullptr),
    m_allocator(nullptr)
{
    // Empty.
}

template <hmem::Handleable UnderlyingType>
hmem::Handle<UnderlyingType>::Handle(const Handle<UnderlyingType>& rhs) {
    *this = std::move(Handle::acquire_existing(rhs));
}

template <hmem::Handleable UnderlyingType>
hmem::Handle<UnderlyingType>& hmem::Handle<UnderlyingType>::operator=(const Handle<UnderlyingType>& rhs) {
    *this = std::move(Handle::acquire_existing(rhs));

    return *this;
}

template <hmem::Handleable UnderlyingType>
hmem::Handle<UnderlyingType>::Handle(Handle<UnderlyingType>&& rhs) :
    m_data(rhs.m_data),
    m_allocator(rhs.m_allocator)
{
    rhs.m_data = nullptr;
}

template <hmem::Handleable UnderlyingType>
hmem::Handle<UnderlyingType>& hmem::Handle<UnderlyingType>::operator=(Handle<UnderlyingType>&& rhs) {
    m_data = rhs.m_data;
    m_allocator = rhs.m_allocator;

    rhs.m_data = nullptr;

    return *this;
}

template <hmem::Handleable UnderlyingType>
bool hmem::Handle<UnderlyingType>::release() {
    if (!m_data) return false;

    HandledAliveState releasing_prev_state = HandledAliveState::ALIVE;
    m_data->alive_state.compare_exchange_strong(releasing_prev_state, HandledAliveState::RELEASING);

    switch (releasing_prev_state) {
    case HandledAliveState::ALIVE:
    case HandledAliveState::RELEASING:
        i32 ref_count = m_data->ref_count.fetch_sub(1);

        if (ref_count - 1 == 0) {
            HandledAliveState dying_prev_state = HandledAliveState::RELEASING;
            m_data->alive_state.compare_exchange_strong(dying_prev_state, HandledAliveState::DYING);

            switch (dying_prev_state) {
            case HandledAliveState::ALIVE:
            case HandledAliveState::ACQUIRING:
                debug_printf("Found handle in non-releasing state when ref count hit zero.");
                assert(false);
                break;
            case HandledAliveState::RELEASING:
                m_allocator->deallocate(*this);
                break;
            case HandledAliveState::DYING:
                // Someone else is deallocating this resource.
                break;
            }
        }

        m_data->alive_state.compare_exchange_strong(HandledAliveState::RELEASING, HandledAliveState::ALIVE);
        break;
    case HandledAliveState::ACQUIRING:
        m_data->ref_count -= 1;
        break;
    case HandledAliveState::DYING:
        // Probably shouldn't reach here, but hey-ho.
        debug_printf("Resource was being deallocated when we tried to release a handle.");
        break;
    }

    return m_allocator->release(*this);
}

template <hmem::Handleable UnderlyingType>
UnderlyingType& hmem::Handle<UnderlyingType>::operator*() {
    return *m_data;
}

template <hmem::Handleable UnderlyingType>
const UnderlyingType& hmem::Handle<UnderlyingType>::operator*() const {
    return *m_data;
}

template <hmem::Handleable UnderlyingType>
UnderlyingType* hmem::Handle<UnderlyingType>::operator->() {
    return m_data;
}

template <hmem::Handleable UnderlyingType>
const UnderlyingType* hmem::Handle<UnderlyingType>::operator->() const {
    return m_data;
}

template <hmem::Handleable UnderlyingType>
bool hmem::Handle<UnderlyingType>::operator==(std::nullptr_t possible_nullptr) {
    return m_data == nullptr;
}

template <hmem::Handleable UnderlyingType>
bool hmem::Handle<UnderlyingType>::operator==(const Handle<UnderlyingType>& handle) {
    return m_data == handle.m_data;
}

template <hmem::Handleable UnderlyingType>
bool hmem::Handle<UnderlyingType>::operator!=(std::nullptr_t possible_nullptr) {
    return !(*this == possible_nullptr);
}

template <hmem::Handleable UnderlyingType>
bool hmem::Handle<UnderlyingType>::operator!=(const Handle<UnderlyingType>& handle) {
    return !(*this == handle);
}

template <hmem::Handleable UnderlyingType>
hmem::Handle<UnderlyingType> hmem::Handle<UnderlyingType>::acquire_existing(const Handle<UnderlyingType>& handle) {
    Handle<UnderlyingType> new_handle;

    new_handle.m_data      = handle.m_data;
    new_handle.m_allocator = handle.m_allocator;

    if (new_handle == nullptr)
        return new_handle;

    HandledAliveState prev_state = HandledAliveState::ALIVE;
    new_handle.m_data->alive_state.compare_exchange_strong(prev_state, HandledAliveState::ACQUIRING);

    switch (prev_state) {
    case HandledAliveState::ALIVE:
        new_handle.m_data->ref_count += 1;
        new_handle.m_data->alive_state.compare_exchange_strong(HandledAliveState::ACQUIRING, HandledAliveState::ALIVE);
        break;
    case HandledAliveState::ACQUIRING:
    case HandledAliveState::RELEASING:
        new_handle.m_data->ref_count += 1;
        break;
    case HandledAliveState::DYING:
        // Allocate new instance of resource.
        Handle<UnderlyingType> allocated_handle = new_handle.m_allocator->acquire();

        // Just copy pointer to allocated resource, fine as long as we make
        // sure we don't release our new resource immediately!
        new_handle.m_data = allocated_handle.m_data;
        allocated_handle.m_data = nullptr;

        break;
    }

    return new_handle;
}

template <hmem::Handleable UnderlyingType>
hmem::Handle<UnderlyingType>::Handle(UnderlyingType* data, Allocator<UnderlyingType>* allocator) :
    m_data(data),
    m_allocator(allocator)
{
    data->ref_count.store(1, std::memory_order_release);
    data->alive_state.store(HandledAliveState::ALIVE, std::memory_order_release);
}

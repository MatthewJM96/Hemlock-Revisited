template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::iterator::iterator(
    StackAllocRingBuffer<Type, Capacity>* buffer /*= nullptr*/, size_t cursor /*= 0*/
) :
    m_buffer(buffer), m_cursor(cursor) {
    // Empty
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator::const_iterator(
    StackAllocRingBuffer<Type, Capacity>* buffer /*= nullptr*/, size_t cursor /*= 0*/
) :
    m_buffer(buffer), m_cursor(cursor) {
    // Empty
}

template <typename Type, size_t Capacity>
Type& hmem::StackAllocRingBuffer<Type, Capacity>::iterator::operator*() {
    return m_buffer[m_cursor];
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::iterator&
hmem::StackAllocRingBuffer<Type, Capacity>::iterator::operator++() {
    ++m_cursor;
    return *this;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::iterator
hmem::StackAllocRingBuffer<Type, Capacity>::iterator::operator++(int) {
    iterator tmp = *this;

    ++m_cursor;

    return tmp;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::iterator&
hmem::StackAllocRingBuffer<Type, Capacity>::iterator::operator--() {
    --m_cursor;
    return *this;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::iterator
hmem::StackAllocRingBuffer<Type, Capacity>::iterator::operator--(int) {
    iterator tmp = *this;

    --m_cursor;

    return tmp;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::iterator&
hmem::StackAllocRingBuffer<Type, Capacity>::iterator::operator+(int offset) {
    m_cursor += offset;
    m_cursor %= m_buffer->size();

    return *this;
}

template <typename Type, size_t Capacity>
const Type& hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator::operator*() {
    return m_buffer[m_cursor];
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator&
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator::operator++() {
    ++m_cursor;
    return *this;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator::operator++(int) {
    const_iterator tmp = *this;

    ++m_cursor;

    return tmp;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator&
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator::operator--() {
    --m_cursor;
    return *this;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator::operator--(int) {
    const_iterator tmp = *this;

    --m_cursor;

    return tmp;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator&
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator::operator+(int offset) {
    m_cursor += offset;
    m_cursor %= m_buffer->size();

    return *this;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::StackAllocRingBuffer() :
    m_start(0), m_end(0) {
    // Empty.
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::StackAllocRingBuffer(
    const StackAllocRingBuffer& rhs
) :
    m_data(rhs.m_data), m_start(rhs.m_start), m_end(rhs.m_end) {
    // Empty.
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::StackAllocRingBuffer(const Underlying& rhs
) :
    m_data(rhs), m_start(0), m_end(m_data.size() - 1) {
    // Empty.
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::StackAllocRingBuffer(Underlying&& rhs) :
    m_data(std::move(rhs)), m_start(0), m_end(m_data.size() - 1) {
    // Empty.
}

template <typename Type, size_t Capacity>
template <typename... Args>
hmem::StackAllocRingBuffer<Type, Capacity>::StackAllocRingBuffer(Args&&... args) :
    m_start(0), m_end(Capacity - 1) {
    std::fill_n(m_data.data(), Capacity, Type(std::forward<Args>(args)...));
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>&
hmem::StackAllocRingBuffer<Type, Capacity>::operator=(const StackAllocRingBuffer& rhs) {
    m_data  = rhs.m_data;
    m_start = rhs.m_start;
    m_end   = rhs.m_end;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>&
hmem::StackAllocRingBuffer<Type, Capacity>::operator=(StackAllocRingBuffer&& rhs) {
    m_data  = rhs.m_data;
    m_start = rhs.m_start;
    m_end   = rhs.m_end;

    rhs.m_start = 0;
    rhs.m_end   = 0;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>&
hmem::StackAllocRingBuffer<Type, Capacity>::operator=(const Underlying& rhs) {
    m_data  = rhs;
    m_start = 0;
    m_end   = m_data.size() - 1;
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>&
hmem::StackAllocRingBuffer<Type, Capacity>::operator=(Underlying&& rhs) {
    m_data  = std::move(rhs);
    m_start = 0;
    m_end   = m_data.size() - 1;
}

template <typename Type, size_t Capacity>
Type& hmem::StackAllocRingBuffer<Type, Capacity>::operator[](size_t index) {
    return m_data[(index + m_start) % Capacity];
}

template <typename Type, size_t Capacity>
const Type& hmem::StackAllocRingBuffer<Type, Capacity>::operator[](size_t index) const {
    return m_data[(index + m_start) % Capacity];
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::iterator
hmem::StackAllocRingBuffer<Type, Capacity>::begin() {
    return iterator(this, 0);
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator
hmem::StackAllocRingBuffer<Type, Capacity>::begin() const {
    return const_iterator(this, 0);
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::iterator
hmem::StackAllocRingBuffer<Type, Capacity>::end() {
    return iterator(this, size());
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator
hmem::StackAllocRingBuffer<Type, Capacity>::end() const {
    return const_iterator(this, size());
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::iterator
hmem::StackAllocRingBuffer<Type, Capacity>::rbegin() {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl
    // of -- on iterator suffice.
    return iterator(this, size() - 1);
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator
hmem::StackAllocRingBuffer<Type, Capacity>::rbegin() const {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl
    // of -- on iterator suffice.
    return const_iterator(this, size() - 1);
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::iterator
hmem::StackAllocRingBuffer<Type, Capacity>::rend() {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl
    // of -- on iterator suffice.
    return iterator(this, size());
}

template <typename Type, size_t Capacity>
hmem::StackAllocRingBuffer<Type, Capacity>::const_iterator
hmem::StackAllocRingBuffer<Type, Capacity>::rend() const {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl
    // of -- on iterator suffice.
    return const_iterator(this, size());
}

template <typename Type, size_t Capacity>
template <typename... Args>
void hmem::StackAllocRingBuffer<Type, Capacity>::emplace_back(Args&&... args) {
    if (m_end == Capacity - 1) {
        m_data[0] = Type(std::forward<Args>(args)...);

        m_end = 0;
        if (m_start == 0) m_start = 1;
    } else {
        m_data[++m_end] = Type(std::forward<Args>(args)...);

        if (m_start == m_end) {
            if (m_start == Capacity - 1) {
                m_start = Capacity - 2;
            } else {
                m_start -= 1;
            }
        }
    }
}

template <typename Type, size_t Capacity>
template <typename... Args>
void hmem::StackAllocRingBuffer<Type, Capacity>::emplace_front(Args&&... args) {
    if (m_start == 0) {
        m_data[Capacity - 1] = Type(std::forward<Args>(args)...);

        m_start = Capacity - 1;
        if (m_end == Capacity - 1) m_end = Capacity - 2;
    } else {
        m_data[m_start - 1] = Type(std::forward<Args>(args)...);

        m_start -= 1;
        if (m_end == m_start) {
            if (m_end == 0) {
                m_end = Capacity - 1;
            } else {
                m_end -= 1;
            }
        }
    }
}

template <typename Type, size_t Capacity>
void hmem::StackAllocRingBuffer<Type, Capacity>::push_back(const Type& value) {
    if (m_end == Capacity - 1) {
        m_data[0] = value;

        m_end = 0;
        if (m_start == 0) m_start = 1;
    } else {
        m_data[++m_end] = value;

        if (m_start == m_end) {
            if (m_start == Capacity - 1) {
                m_start = Capacity - 2;
            } else {
                m_start -= 1;
            }
        }
    }
}

template <typename Type, size_t Capacity>
void hmem::StackAllocRingBuffer<Type, Capacity>::push_back(Type&& value) {
    if (m_end == Capacity - 1) {
        m_data[0] = std::forward<Type>(value);

        m_end = 0;
        if (m_start == 0) m_start = 1;
    } else {
        m_data[++m_end] = std::forward<Type>(value);

        if (m_start == m_end) {
            if (m_start == Capacity - 1) {
                m_start = Capacity - 2;
            } else {
                m_start -= 1;
            }
        }
    }
}

template <typename Type, size_t Capacity>
void hmem::StackAllocRingBuffer<Type, Capacity>::push_front(const Type& value) {
    if (m_start == 0) {
        m_data[Capacity - 1] = value;

        m_start = Capacity - 1;
        if (m_end == Capacity - 1) m_end = Capacity - 2;
    } else {
        m_data[--m_start] = value;

        if (m_end == m_start) {
            if (m_end == 0) {
                m_end = Capacity - 1;
            } else {
                m_end -= 1;
            }
        }
    }
}

template <typename Type, size_t Capacity>
void hmem::StackAllocRingBuffer<Type, Capacity>::push_front(Type&& value) {
    if (m_start == 0) {
        m_data[Capacity - 1] = std::forward<Type>(value);

        m_start = Capacity - 1;
        if (m_end == Capacity - 1) m_end = Capacity - 2;
    } else {
        m_data[m_start - 1] = std::forward<Type>(value);

        m_start -= 1;
        if (m_end == m_start) {
            if (m_end == 0) {
                m_end = Capacity - 1;
            } else {
                m_end -= 1;
            }
        }
    }
}

template <typename Type, size_t Capacity>
void hmem::StackAllocRingBuffer<Type, Capacity>::erase_back() {
    assert(!empty());

    m_data[m_end].~Type();

    if (m_end == 0) {
        m_end = Capacity - 1;
    } else {
        m_end -= 1;
    }
}

template <typename Type, size_t Capacity>
void hmem::StackAllocRingBuffer<Type, Capacity>::erase_front() {
    assert(!empty());

    m_data[m_start].~Type();

    if (m_start == Capacity - 1) {
        m_start = 0;
    } else {
        m_start += 1;
    }
}

template <typename Type, size_t Capacity>
Type&& hmem::StackAllocRingBuffer<Type, Capacity>::pop_back() {
    assert(!empty());

    Type* tmp = &m_data[m_end];

    if (m_end == 0) {
        m_end = Capacity - 1;
    } else {
        m_end -= 1;
    }

    return std::move(*tmp);
}

template <typename Type, size_t Capacity>
Type&& hmem::StackAllocRingBuffer<Type, Capacity>::pop_front() {
    assert(!empty());

    Type* tmp = &m_data[m_start];

    if (m_start == Capacity - 1) {
        m_start = 0;
    } else {
        m_start += 1;
    }

    return std::move(*tmp);
}

template <typename Type, size_t Capacity>
bool hmem::StackAllocRingBuffer<Type, Capacity>::empty() const {
    // Only true at initialisation with:
    //   m_start == m_end == 0
    // or after popping the last element.
    return m_start == m_end;
}

template <typename Type, size_t Capacity>
bool hmem::StackAllocRingBuffer<Type, Capacity>::full() const {
    return (m_end < m_start) || (m_end == Capacity - 1);
}

template <typename Type, size_t Capacity>
size_t hmem::StackAllocRingBuffer<Type, Capacity>::size() const {
    if (m_end < m_start) return Capacity;

    return m_end - m_start;
}

template <typename Type, size_t Capacity>
constexpr size_t hmem::StackAllocRingBuffer<Type, Capacity>::capacity() const {
    return Capacity;
}

template <typename Type, size_t Capacity>
void hmem::StackAllocRingBuffer<Type, Capacity>::clear() {
    m_end = m_start;
}

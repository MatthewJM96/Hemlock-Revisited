template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator::iterator(FixedCapacityRingBuffer<Type, Capacity>* buffer /*= nullptr*/, size_t cursor /*= 0*/) :
    m_buffer(buffer), m_cursor(cursor) {
    // Empty
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator::const_iterator(FixedCapacityRingBuffer<Type, Capacity>* buffer /*= nullptr*/, size_t cursor /*= 0*/) :
    m_buffer(buffer), m_cursor(cursor) {
    // Empty
}

template <typename Type, size_t Capacity>
Type& hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator::operator*() {
    return m_buffer[m_cursor];
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator& hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator::operator++() {
    ++m_cursor;
    return *this;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator::operator++(int) {
    iterator tmp = *this;

    ++m_cursor;

    return tmp;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator& hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator::operator--() {
    --m_cursor;
    return *this;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator::operator--(int) {
    iterator tmp = *this;

    --m_cursor;

    return tmp;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator& hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator::operator+(int offset) {
    m_cursor += offset;
    m_cursor %= m_buffer->size();

    return *this;
}

template <typename Type, size_t Capacity>
const Type& hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator::operator*() {
    return m_buffer[m_cursor];
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator& hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator::operator++() {
    ++m_cursor;
    return *this;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator::operator++(int) {
    const_iterator tmp = *this;

    ++m_cursor;

    return tmp;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator& hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator::operator--() {
    --m_cursor;
    return *this;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator::operator--(int) {
    const_iterator tmp = *this;

    --m_cursor;

    return tmp;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator& hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator::operator+(int offset) {
    m_cursor += offset;
    m_cursor %= m_buffer->size();

    return *this;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::FixedCapacityRingBuffer() : m_data(Underlying(Capacity)), m_start(0), m_end(0) {
    // Empty.
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::FixedCapacityRingBuffer(const FixedCapacityRingBuffer& rhs) : m_data(rhs.m_data), m_start(rhs.m_start), m_end(rhs.m_end) {
    // Empty.
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::FixedCapacityRingBuffer(FixedCapacityRingBuffer&& rhs) : m_data(std::move(rhs.m_data)), m_start(rhs.m_start), m_end(rhs.m_end) {
    // Empty.
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::FixedCapacityRingBuffer(const Underlying& rhs) : m_data(rhs), m_start(0), m_end(m_data.size() - 1) {
    // Empty.
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::FixedCapacityRingBuffer(Underlying&& rhs) : m_data(std::move(rhs)), m_start(0), m_end(m_data.size() - 1) {
    // Empty.
}

template <typename Type, size_t Capacity>
template <typename ...Args>
hmem::FixedCapacityRingBuffer<Type, Capacity>::FixedCapacityRingBuffer(Args&&... args) : m_data(Underlying(Capacity, Type(std::forward<Args>(args)...))), m_start(0), m_end(Capacity - 1) {
    // Empty.
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>& hmem::FixedCapacityRingBuffer<Type, Capacity>::operator=(const FixedCapacityRingBuffer& rhs) {
    // TODO(Matthew): why do I suspect this doesn't do a full copy of the array? It should... trust issues
    m_data = rhs.m_data;
    m_start = rhs.m_start;
    m_end = rhs.m_end;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>& hmem::FixedCapacityRingBuffer<Type, Capacity>::operator=(FixedCapacityRingBuffer&& rhs) {
    // TODO(Matthew): std::array doesn't itself call new, so m_data is on the stack and hence move wouldn't optimise anything.
    //                perhaps we should consider small-buffer optimisation but otherwise not use std::array.
    m_data = std::move(rhs.m_data);
    m_start = rhs.m_start;
    m_end = rhs.m_end;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>& hmem::FixedCapacityRingBuffer<Type, Capacity>::operator=(const Underlying& rhs) {
    m_data = rhs;
    m_start = 0;
    m_end = m_data.size() - 1;
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>& hmem::FixedCapacityRingBuffer<Type, Capacity>::operator=(Underlying&& rhs) {
    m_data = std::move(rhs);
    m_start = 0;
    m_end = m_data.size() - 1;
}

template <typename Type, size_t Capacity>
Type& hmem::FixedCapacityRingBuffer<Type, Capacity>::operator[](size_t index) {
    return m_data[(index + m_start) % Capacity];
}

template <typename Type, size_t Capacity>
const Type& hmem::FixedCapacityRingBuffer<Type, Capacity>::operator[](size_t index) const {
    return m_data[(index + m_start) % Capacity];
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::begin() {
    return iterator(this, 0);
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::begin() const {
    return const_iterator(this, 0);
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::end() {
    return iterator(this, size());
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::end() const {
    return const_iterator(this, size());
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::rbegin() {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl of -- on iterator suffice.
    return iterator(this, size() - 1);
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::rbegin() const {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl of -- on iterator suffice.
    return const_iterator(this, size() - 1);
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::rend() {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl of -- on iterator suffice.
    return iterator(this, size());
}

template <typename Type, size_t Capacity>
hmem::FixedCapacityRingBuffer<Type, Capacity>::const_iterator hmem::FixedCapacityRingBuffer<Type, Capacity>::rend() const {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl of -- on iterator suffice.
    return const_iterator(this, size());
}

template <typename Type, size_t Capacity>
template <typename ...Args>
void hmem::FixedCapacityRingBuffer<Type, Capacity>::emplace_back(Args&&... args) {
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
template <typename ...Args>
void hmem::FixedCapacityRingBuffer<Type, Capacity>::emplace_front(Args&&... args) {
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
void hmem::FixedCapacityRingBuffer<Type, Capacity>::push_back(const Type& value) {
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
void hmem::FixedCapacityRingBuffer<Type, Capacity>::push_back(Type&& value) {
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
void hmem::FixedCapacityRingBuffer<Type, Capacity>::push_front(const Type& value) {
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
void hmem::FixedCapacityRingBuffer<Type, Capacity>::push_front(Type&& value) {
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
void hmem::FixedCapacityRingBuffer<Type, Capacity>::erase_back() {
    assert(!empty());

    m_data[m_end].~Type();

    if (m_end == 0) {
        m_end = Capacity - 1;
    } else {
        m_end -= 1;
    }
}

template <typename Type, size_t Capacity>
void hmem::FixedCapacityRingBuffer<Type, Capacity>::erase_front() {
    assert(!empty());

    m_data[m_start].~Type();

    if (m_start == Capacity - 1) {
        m_start = 0;
    } else {
        m_start += 1;
    }
}

template <typename Type, size_t Capacity>
Type&& hmem::FixedCapacityRingBuffer<Type, Capacity>::pop_back() {
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
Type&& hmem::FixedCapacityRingBuffer<Type, Capacity>::pop_front() {
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
bool hmem::FixedCapacityRingBuffer<Type, Capacity>::empty() const {
    // Only true at initialisation with:
    //   m_start == m_end == 0
    // or after popping the last element.
    return m_start == m_end;
}

template <typename Type, size_t Capacity>
bool hmem::FixedCapacityRingBuffer<Type, Capacity>::full() const {
    return (m_end < m_start) || (m_end == Capacity - 1);
}

template <typename Type, size_t Capacity>
size_t hmem::FixedCapacityRingBuffer<Type, Capacity>::size() const {
    if (m_end < m_start) return Capacity;

    return m_end - m_start;
}

template <typename Type, size_t Capacity>
constexpr size_t hmem::FixedCapacityRingBuffer<Type, Capacity>::capacity() const {
    return Capacity;
}

template <typename Type, size_t Capacity>
void hmem::FixedCapacityRingBuffer<Type, Capacity>::clear() {
    m_end = m_start;
}

template <typename Type>
hmem::RingBuffer<Type>::iterator::iterator(RingBuffer<Type>* buffer /*= nullptr*/, size_t cursor /*= 0*/) :
    m_buffer(buffer), m_cursor(cursor) {
    // Empty
}

template <typename Type>
hmem::RingBuffer<Type>::const_iterator::const_iterator(RingBuffer<Type>* buffer /*= nullptr*/, size_t cursor /*= 0*/) :
    m_buffer(buffer), m_cursor(cursor) {
    // Empty
}

template <typename Type>
Type& hmem::RingBuffer<Type>::iterator::operator*() {
    return m_buffer[m_cursor];
}

template <typename Type>
hmem::RingBuffer<Type>::iterator& hmem::RingBuffer<Type>::iterator::operator++() {
    ++m_cursor;
    return *this;
}

template <typename Type>
hmem::RingBuffer<Type>::iterator hmem::RingBuffer<Type>::iterator::operator++(int) {
    iterator tmp = *this;

    ++m_cursor;

    return tmp;
}

template <typename Type>
hmem::RingBuffer<Type>::iterator& hmem::RingBuffer<Type>::iterator::operator--() {
    --m_cursor;
    return *this;
}

template <typename Type>
hmem::RingBuffer<Type>::iterator hmem::RingBuffer<Type>::iterator::operator--(int) {
    iterator tmp = *this;

    --m_cursor;

    return tmp;
}

template <typename Type>
hmem::RingBuffer<Type>::iterator& hmem::RingBuffer<Type>::iterator::operator+(int offset) {
    m_cursor += offset;
    m_cursor %= m_buffer->size();

    return *this;
}

template <typename Type>
const Type& hmem::RingBuffer<Type>::const_iterator::operator*() {
    return m_buffer[m_cursor];
}

template <typename Type>
hmem::RingBuffer<Type>::const_iterator& hmem::RingBuffer<Type>::const_iterator::operator++() {
    ++m_cursor;
    return *this;
}

template <typename Type>
hmem::RingBuffer<Type>::const_iterator hmem::RingBuffer<Type>::const_iterator::operator++(int) {
    const_iterator tmp = *this;

    ++m_cursor;

    return tmp;
}

template <typename Type>
hmem::RingBuffer<Type>::const_iterator& hmem::RingBuffer<Type>::const_iterator::operator--() {
    --m_cursor;
    return *this;
}

template <typename Type>
hmem::RingBuffer<Type>::const_iterator hmem::RingBuffer<Type>::const_iterator::operator--(int) {
    const_iterator tmp = *this;

    --m_cursor;

    return tmp;
}

template <typename Type>
hmem::RingBuffer<Type>::const_iterator& hmem::RingBuffer<Type>::const_iterator::operator+(int offset) {
    m_cursor += offset;
    m_cursor %= m_buffer->size();

    return *this;
}

template <typename Type>
hmem::RingBuffer<Type>::RingBuffer() {
    // Empty.
}

template <typename Type>
hmem::RingBuffer<Type>::RingBuffer(const RingBuffer<Type>& rhs) {

}

template <typename Type>
hmem::RingBuffer<Type>::RingBuffer(RingBuffer<Type>&& rhs) {

}

template <typename Type>
template <typename ...Args>
hmem::RingBuffer<Type>::RingBuffer(Args&&... args) {

}

template <typename Type>
hmem::RingBuffer<Type>& hmem::RingBuffer<Type>::operator=(const RingBuffer<Type>& rhs) {

}

template <typename Type>
hmem::RingBuffer<Type>& hmem::RingBuffer<Type>::operator=(RingBuffer<Type>&& rhs) {

}

template <typename Type>
Type& hmem::RingBuffer<Type>::operator[](size_t index) {
    return m_data[(index + m_start) % m_capacity];
}

template <typename Type>
const Type& hmem::RingBuffer<Type>::operator[](size_t index) const {
    return m_data[(index + m_start) % m_capacity];
}

template <typename Type>
hmem::RingBuffer<Type>::iterator hmem::RingBuffer<Type>::begin() {
    return iterator(this, 0);
}

template <typename Type>
hmem::RingBuffer<Type>::const_iterator hmem::RingBuffer<Type>::begin() const {
    return const_iterator(this, 0);
}

template <typename Type>
hmem::RingBuffer<Type>::iterator hmem::RingBuffer<Type>::end() {
    return iterator(this, size());
}

template <typename Type>
hmem::RingBuffer<Type>::const_iterator hmem::RingBuffer<Type>::end() const {
    return const_iterator(this, size());
}

template <typename Type>
hmem::RingBuffer<Type>::iterator hmem::RingBuffer<Type>::rbegin() {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl of -- on iterator suffice.
    return iterator(this, size() - 1);
}

template <typename Type>
hmem::RingBuffer<Type>::const_iterator hmem::RingBuffer<Type>::rbegin() const {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl of -- on iterator suffice.
    return const_iterator(this, size() - 1);
}

template <typename Type>
hmem::RingBuffer<Type>::iterator hmem::RingBuffer<Type>::rend() {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl of -- on iterator suffice.
    return iterator(this, size());
}

template <typename Type>
hmem::RingBuffer<Type>::const_iterator hmem::RingBuffer<Type>::rend() const {
    // TODO(Matthew): do we need a reverse iterator specifically for this? or does impl of -- on iterator suffice.
    return const_iterator(this, size());
}

template <typename Type>
template <typename ...Args>
void hmem::RingBuffer<Type>::emplace_back(Args&&... args) {
    if (m_end == m_capacity - 1) {
        m_data[0] = Type(std::forward<Args>(args)...);

        m_end = 0;
        if (m_start == 0) m_start = 1;
    } else {
        m_data[++m_end] = Type(std::forward<Args>(args)...);

        if (m_start == m_end) {
            if (m_start == m_capacity - 1) {
                m_start = m_capacity - 2;
            } else {
                m_start -= 1;
            }
        }
    }
}

template <typename Type>
template <typename ...Args>
void hmem::RingBuffer<Type>::emplace_front(Args&&... args) {
    if (m_start == 0) {
        m_data[m_capacity - 1] = Type(std::forward<Args>(args)...);

        m_start = m_capacity - 1;
        if (m_end == m_capacity - 1) m_end = m_capacity - 2;
    } else {
        m_data[m_start - 1] = Type(std::forward<Args>(args)...);

        m_start -= 1;
        if (m_end == m_start) {
            if (m_end == 0) {
                m_end = m_capacity - 1;
            } else {
                m_end -= 1;
            }
        }
    }
}

template <typename Type>
void hmem::RingBuffer<Type>::push_back(const Type& value) {
    if (m_end == m_capacity - 1) {
        m_data[0] = value;

        m_end = 0;
        if (m_start == 0) m_start = 1;
    } else {
        m_data[++m_end] = value;

        if (m_start == m_end) {
            if (m_start == m_capacity - 1) {
                m_start = m_capacity - 2;
            } else {
                m_start -= 1;
            }
        }
    }
}

template <typename Type>
void hmem::RingBuffer<Type>::push_back(Type&& value) {
    if (m_end == m_capacity - 1) {
        m_data[0] = std::forward<Type>(value);

        m_end = 0;
        if (m_start == 0) m_start = 1;
    } else {
        m_data[++m_end] = std::forward<Type>(value);

        if (m_start == m_end) {
            if (m_start == m_capacity - 1) {
                m_start = m_capacity - 2;
            } else {
                m_start -= 1;
            }
        }
    }
}

template <typename Type>
void hmem::RingBuffer<Type>::push_front(const Type& value) {
    if (m_start == 0) {
        m_data[m_capacity - 1] = value;

        m_start = m_capacity - 1;
        if (m_end == m_capacity - 1) m_end = m_capacity - 2;
    } else {
        m_data[m_start - 1] = value;

        m_start -= 1;
        if (m_end == m_start) {
            if (m_end == 0) {
                m_end = m_capacity - 1;
            } else {
                m_end -= 1;
            }
        }
    }
}

template <typename Type>
void hmem::RingBuffer<Type>::push_front(Type&& value) {
    if (m_start == 0) {
        m_data[m_capacity - 1] = std::forward<Type>(value);

        m_start = m_capacity - 1;
        if (m_end == m_capacity - 1) m_end = m_capacity - 2;
    } else {
        m_data[m_start - 1] = std::forward<Type>(value);

        m_start -= 1;
        if (m_end == m_start) {
            if (m_end == 0) {
                m_end = m_capacity - 1;
            } else {
                m_end -= 1;
            }
        }
    }
}

template <typename Type>
void hmem::RingBuffer<Type>::erase_back() {
    assert(!empty());

    m_data[m_end].~Type();

    if (m_end == 0) {
        m_end = m_capacity - 1;
    } else {
        m_end -= 1;
    }
}

template <typename Type>
void hmem::RingBuffer<Type>::erase_front() {
    assert(!empty());

    m_data[m_start].~Type();

    if (m_start == m_capacity - 1) {
        m_start = 0;
    } else {
        m_start += 1;
    }
}

template <typename Type>
Type&& hmem::RingBuffer<Type>::pop_back() {
    assert(!empty());

    Type* tmp = &m_data[m_end];

    if (m_end == 0) {
        m_end = m_capacity - 1;
    } else {
        m_end -= 1;
    }

    return std::move(*tmp);
}

template <typename Type>
Type&& hmem::RingBuffer<Type>::pop_front() {
    assert(!empty());

    Type* tmp = &m_data[m_start];

    if (m_start == m_capacity - 1) {
        m_start = 0;
    } else {
        m_start += 1;
    }

    return std::move(*tmp);
}

template <typename Type>
bool hmem::RingBuffer<Type>::empty() const {
    // Only true at initialisation with:
    //   m_start == m_end == 0
    // or after popping the last element.
    return m_start == m_end;
}

template <typename Type>
bool hmem::RingBuffer<Type>::full() const {
    return (m_end < m_start) || (m_end == m_capacity - 1);
}

template <typename Type>
size_t hmem::RingBuffer<Type>::size() const {
    if (m_end < m_start) return m_capacity;

    return m_end - m_start;
}

template <typename Type>
size_t hmem::RingBuffer<Type>::capacity() const {
    return m_capacity;
}

template <typename Type>
void hmem::RingBuffer<Type>::resize(size_t new_capacity) {
    size_t head_length = m_capacity - m_start;
    size_t tail_length = m_end + 1;

    Type* tmp = new Type[new_capacity];

    // Perform resize sensitive to if we are resizing to more or less than current
    // size.
    if (new_capacity > size()) {
        // If we have wrapped around and are currently not linear, then linearise while
        // performing the resize.
        if (m_start == 0) {
            std::memcpy(tmp, m_data, tail_length * sizeof(Type));
        } else {
            std::memcpy(tmp, m_data + m_start, head_length * sizeof(Type));
            std::memcpy(tmp + head_length, m_data, tail_length * sizeof(Type));
        }
    } else {
        // If we have wrapped around and are currently not linear, then linearise while
        // performing the resize. We only need to do one copy if linear, or if not
        // linear but new capacity only allows a linear portion of the current buffer.
        if (m_start == 0 || new_capacity < head_length) {
            std::memcpy(tmp, m_data, new_capacity * sizeof(Type));
        } else {
            std::memcpy(tmp, m_data + m_start, head_length * sizeof(Type));
            std::memcpy(tmp + head_length, m_data, (new_capacity - head_length) * sizeof(Type));
        }
    }

    m_data = tmp;
    m_capacity = new_capacity;
}

template <typename Type>
void hmem::RingBuffer<Type>::clear() {
    m_end = m_start;
}

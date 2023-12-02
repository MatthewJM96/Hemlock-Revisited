#ifndef __hemlock_memory_ring_buffer_dynamic_hpp
#define __hemlock_memory_ring_buffer_dynamic_hpp

namespace hemlock {
    namespace memory {
        template <typename Type>
        class RingBuffer {
        public:
            struct iterator {
                iterator(RingBuffer<Type>* buffer = nullptr, size_t cursor = 0);

                Type& operator*();

                iterator& operator++();
                iterator operator++(int);

                iterator& operator--();
                iterator operator--(int);

                iterator& operator+(int offset);
            protected:
                RingBuffer<Type>* m_buffer;
                size_t m_cursor;
            };
            struct const_iterator {
                const_iterator(RingBuffer<Type>* buffer = nullptr, size_t cursor = 0);

                const Type& operator*();

                const_iterator& operator++();
                const_iterator operator++(int);

                const_iterator& operator--();
                const_iterator operator--(int);

                const_iterator& operator+(int offset);
            protected:
                const RingBuffer<Type>* m_buffer;
                size_t m_cursor;
            };

            RingBuffer();
            RingBuffer(const RingBuffer& rhs);
            RingBuffer(RingBuffer&& rhs);

            template <typename ...Args>
            RingBuffer(Args&&... args);

            RingBuffer& operator=(const RingBuffer& rhs);
            RingBuffer& operator=(RingBuffer&& rhs);

            Type& operator[](size_t index);
            const Type& operator[](size_t index) const;

            iterator begin();
            const_iterator begin() const;

            iterator end();
            const_iterator end() const;

            iterator rbegin();
            const_iterator rbegin() const;

            iterator rend();
            const_iterator rend() const;

            template <typename ...Args>
            void emplace_back(Args&&... args);

            template <typename ...Args>
            void emplace_front(Args&&... args);

            void push_back(const Type& value);
            void push_back(Type&& value);

            void push_front(const Type& value);
            void push_front(Type&& value);

            void erase_back();
            void erase_front();

            Type&& pop_back();
            Type&& pop_front();

            bool empty() const;

            bool full() const;

            size_t size() const;

            size_t capacity() const;

            void resize(size_t new_capacity);

            void clear();
        protected:
            Type* m_data;
            size_t m_start, m_end, m_capacity;
        };
    }  // namespace memory
}  // namespace hemlock
namespace hmem = hemlock::memory;

#include "memory/ring_buffer/dynamic.inl"

#endif  // __hemlock_memory_ring_buffer_dynamic_hpp

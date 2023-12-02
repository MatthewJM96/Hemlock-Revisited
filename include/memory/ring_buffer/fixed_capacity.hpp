#ifndef __hemlock_memory_ring_buffer_fixed_capacity_hpp
#define __hemlock_memory_ring_buffer_fixed_capacity_hpp

namespace hemlock {
    namespace memory {
        template <typename Type, size_t Capacity>
        class FixedCapacityRingBuffer {
        public:
            struct iterator {
                iterator(FixedCapacityRingBuffer<Type, Capacity>* buffer = nullptr, size_t cursor = 0);

                Type& operator*();

                iterator& operator++();
                iterator operator++(int);

                iterator& operator--();
                iterator operator--(int);

                iterator& operator+(int offset);
            protected:
                FixedCapacityRingBuffer<Type, Capacity>* m_buffer;
                size_t m_cursor;
            };
            struct const_iterator {
                const_iterator(FixedCapacityRingBuffer<Type, Capacity>* buffer = nullptr, size_t cursor = 0);

                const Type& operator*();

                const_iterator& operator++();
                const_iterator operator++(int);

                const_iterator& operator--();
                const_iterator operator--(int);

                const_iterator& operator+(int offset);
            protected:
                const FixedCapacityRingBuffer<Type, Capacity>* m_buffer;
                size_t m_cursor;
            };

            using Underlying = std::vector<Type>;

            FixedCapacityRingBuffer();
            FixedCapacityRingBuffer(const FixedCapacityRingBuffer& rhs);
            FixedCapacityRingBuffer(FixedCapacityRingBuffer&& rhs);

            FixedCapacityRingBuffer(const Underlying& rhs);
            FixedCapacityRingBuffer(Underlying&& rhs);

            template <typename ...Args>
            FixedCapacityRingBuffer(Args&&... args);

            FixedCapacityRingBuffer& operator=(const FixedCapacityRingBuffer& rhs);
            FixedCapacityRingBuffer& operator=(FixedCapacityRingBuffer&& rhs);

            FixedCapacityRingBuffer& operator=(const Underlying& rhs);
            FixedCapacityRingBuffer& operator=(Underlying&& rhs);

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

            constexpr size_t capacity() const;

            void clear();
        protected:
            Underlying m_data;
            size_t m_start, m_end;
        };
    }  // namespace memory
}  // namespace hemlock
namespace hmem = hemlock::memory;

#include "memory/ring_buffer/fixed_capacity.inl"

#endif  // __hemlock_memory_ring_buffer_fixed_capacity_hpp

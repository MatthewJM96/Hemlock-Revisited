#ifndef __hemlock_memory_handle_hpp
#define __hemlock_memory_handle_hpp

#include "allocator.hpp"

namespace hemlock {
    namespace memory {
        template <IsHandleable UnderlyingType>
        class Handle {
            friend Allocator<UnderlyingType>;
        public:
            Handle();
            ~Handle() { release(); }

            Handle(const Handle&);
            Handle& operator=(const Handle&);

            Handle(Handle&&);
            Handle& operator=(Handle&&);

            bool release();

            UnderlyingType& operator*();
            const UnderlyingType& operator*() const;

            UnderlyingType* operator->();
            const UnderlyingType* operator->() const;

            bool operator==(std::nullptr_t possible_nullptr);
            bool operator==(const Handle& handle);

            bool operator!=(std::nullptr_t possible_nullptr);
            bool operator!=(const Handle& handle);
        protected:
            static Handle acquire_existing(const Handle& handle);

            Handle(UnderlyingType* data, Allocator<UnderlyingType>* allocator);

            UnderlyingType* m_data;
            Allocator<UnderlyingType>* m_allocator;
        };
    }
}
namespace hmem = hemlock::memory;

#include "handle.inl"

#endif // __hemlock_memory_handle_hpp

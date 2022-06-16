#ifndef __hemlock_memory_allocator_hpp
#define __hemlock_memory_allocator_hpp

namespace hemlock {
    namespace memory {
        enum class HandledAliveState : ui8 {
            ALIVE,
            ACQUIRING,
            RELEASING,
            DYING
        };

        struct Handleable {
            std::atomic<HandledAliveState> alive_state;
            std::atomic<i32>               ref_count;
        };

        template <typename CandidateType>
        concept IsHandleable = std::derived_from<CandidateType, Handleable>;

        template <IsHandleable UnderlyingType>
        class Handle;

        template <typename DataType>
        class Allocator {
        protected:
        public:
            Allocator()  { /* Empty. */ }
            ~Allocator() { /* Empty. */ }

            virtual Handle<DataType> allocate()                            = 0;
            virtual             bool deallocate(Handle<DataType>&& handle) = 0;
        };
    }
}
namespace hmem = hemlock::memory;

#endif // __hemlock_memory_allocator_hpp

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

        // TODO(Matthew): Figure thread-safe impl. Atomic or mutex, not both.
        template <typename CandidateType>
        concept Handleable = requires(CandidateType a) {
            { a.alive_state } -> std::same_as<std::atomic<HandledAliveState>>;
            { a.ref_count   } -> std::same_as<std::atomic<i32>>;
        };

        template <Handleable UnderlyingType>
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

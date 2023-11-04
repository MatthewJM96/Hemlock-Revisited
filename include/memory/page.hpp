#ifndef __hemlock_memory_page_hpp
#define __hemlock_memory_page_hpp

namespace hemlock {
    namespace memory {
        template <typename DataType>
        using Page = DataType*;

        template <typename DataType, size_t MaxFreePages>
            requires (MaxFreePages > 0)
        using Pages = std::array<Page<DataType>, MaxFreePages>;
    }  // namespace memory
}  // namespace hemlock
namespace hmem = hemlock::memory;

#endif  // __hemlock_memory_page_hpp

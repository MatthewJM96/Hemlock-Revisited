#ifndef __hemlock_memory_heterogenous_pager_hpp
#define __hemlock_memory_heterogenous_pager_hpp

#include "pager.hpp"

namespace hemlock {
    namespace memory {
        template <size_t MaxFreePages>
            requires (MaxFreePages > 0)
        using HeterogenousPages = std::unordered_map<size_t, Pages<void, MaxFreePages>>;

        using HeterogenousFreePageCounts = std::unordered_map<size_t, size_t>;

        template <size_t PageSize, size_t MaxFreePages>
            requires (PageSize > 0 && MaxFreePages > 0)
        class HeterogenousPager {
        protected:
            using _HeterogenousPages = HeterogenousPages<MaxFreePages>;
        public:
            HeterogenousPager() { /* Empty. */
            }

            ~HeterogenousPager() { /* Empty. */
            }

            void dispose();

            template <typename DataType>
            Page<DataType> get_page();
            template <typename DataType>
            void free_page(Page<DataType> page);
        protected:
            std::mutex                 m_free_pages_mutex;
            _HeterogenousPages         m_free_pages;
            HeterogenousFreePageCounts m_free_page_counts;
        };
    }  // namespace memory
}  // namespace hemlock
namespace hmem = hemlock::memory;

#include "heterogenous_pager.inl"

#endif  // __hemlock_memory_heterogenous_pager_hpp

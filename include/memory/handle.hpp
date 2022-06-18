#ifndef __hemlock_memory_handle_hpp
#define __hemlock_memory_handle_hpp

namespace hemlock {
    namespace memory {
        template <typename DataType>
        using Handle = std::shared_ptr<DataType>;

        template <typename DataType>
        using WeakHandle = std::weak_ptr<DataType>;

        template <typename DataType, typename Allocator, class... Args>
        Handle<DataType> allocate_handle(const Allocator& allocator, Args&&... args) {
            return std::allocate_shared<DataType>(allocator, std::forward<Args...>(args...));
        }

        template <typename DataType, typename Allocator>
        Handle<DataType> allocate_handle(const Allocator& allocator, std::size_t N) {
            return std::allocate_shared<DataType>(allocator, N);
        }

        template <typename DataType, typename Allocator>
        Handle<DataType> allocate_handle(const Allocator& allocator) {
            return std::allocate_shared<DataType>(allocator);
        }

        template <typename DataType, typename Allocator>
        Handle<DataType> allocate_handle(const Allocator& allocator, std::size_t N,
                                    const std::remove_extent_t<DataType>& u) {
            return std::allocate_shared<DataType>(allocator, N, u);
        }

        template <typename DataType, typename Allocator>
        Handle<DataType> allocate_handle(const Allocator& allocator,
                                    const std::remove_extent_t<DataType>& u) {
            return std::allocate_shared<DataType>(allocator, u);
        }

        template <typename DataType, typename Allocator>
        Handle<DataType> allocate_handle_for_overwrite(const Allocator& allocator) {
            return std::allocate_shared_for_overwrite<DataType>(allocator);
        }

        template <typename DataType, typename Allocator>
        Handle<DataType> allocate_handle_for_overwrite(const Allocator& allocator, std::size_t N) {
            return std::allocate_shared_for_overwrite<DataType>(allocator, N);
        }

        template <typename DataType, class... Args>
        Handle<DataType> make_handle(Args&&... args) {
            return std::make_shared<DataType, Args...>(std::forward<Args...>(args...));
        }

        template <typename DataType>
        Handle<DataType> make_handle(std::size_t N) {
            return std::make_shared<DataType>(N);
        }

        template <typename DataType>
        Handle<DataType> make_handle() {
            return std::make_shared<DataType>();
        }

        template <typename DataType>
        Handle<DataType> make_handle(std::size_t N,
                                    const std::remove_extent_t<DataType>& u) {
            return std::make_shared<DataType>(N, u);
        }

        template <typename DataType>
        Handle<DataType> make_handle(const std::remove_extent_t<DataType>& u) {
            return std::make_shared<DataType>(u);
        }

        template <typename DataType>
        Handle<DataType> make_handle_for_overwrite() {
            return std::make_shared_for_overwrite<DataType>();
        }

        template <typename DataType>
        Handle<DataType> make_handle_for_overwrite(std::size_t N) {
            return std::make_shared_for_overwrite<DataType>(N);
        }
    }
}
namespace hmem = hemlock::memory;

#endif // __hemlock_memory_handle_hpp

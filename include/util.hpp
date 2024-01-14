#ifndef __hemlock_util_hpp
#define __hemlock_util_hpp

namespace hemlock {
    template <typename Func, typename... Types, size_t... Indices, typename... Args>
    void invoke_on_impl(
        std::tuple<Types...>& tuple,
        std::index_sequence<Indices...>,
        size_t idx,
        Func   func,
        Args&&... args
    ) {
        ((void
         )(Indices == idx
           && (std::get<Indices>(tuple).func(std::forward<Args>(args)...), true)),
         ...);
    }

    template <typename Func, typename... Types, typename... Args>
    void invoke_on(std::tuple<Types...>& tuple, size_t idx, Func func, Args&&... args) {
        invoke_on_impl(
            tuple, std::make_index_sequence<sizeof...(Types)>{}, idx, func, args...
        );
    }

    template <typename Func, typename... Types, size_t... Indices, typename... Args>
    void invoke_at_impl(
        std::tuple<Types...>& tuple,
        std::index_sequence<Indices...>,
        size_t idx,
        Func   func,
        Args&&... args
    ) {
        ((void
         )(Indices == idx
           && (func(std::get<Indices>(tuple), std::forward<Args>(args)...), true)),
         ...);
    }

    template <typename Func, typename... Types, typename... Args>
    void invoke_at(std::tuple<Types...>& tuple, size_t idx, Func func, Args&&... args) {
        invoke_at_impl(
            tuple,
            std::make_index_sequence<sizeof...(Types)>{},
            idx,
            func,
            std::forward<Args>(args)...
        );
    }
}  // namespace hemlock

#endif  // __hemlock_util_hpp

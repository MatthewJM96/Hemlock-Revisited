#ifndef __hemlock_basic_concepts_hpp
#define __hemlock_basic_concepts_hpp

#include <type_traits>

namespace hemlock {
    namespace impl {
        template <typename _BitReference>
        concept BitReference = requires(
            _BitReference             bit_reference,
            const _BitReference const_bit_reference,
            bool                       boolean_stub
        ) {
            { static_cast<bool>(bit_reference) };
            {       bit_reference =        bit_reference } -> std::same_as<_BitReference&>;
            {       bit_reference =        boolean_stub  } -> std::same_as<_BitReference&>;
            {       bit_reference ==       bit_reference } -> std::same_as<bool>;
            {       bit_reference <        bit_reference } -> std::same_as<bool>;
            { const_bit_reference ==       bit_reference } -> std::same_as<bool>;
            { const_bit_reference <        bit_reference } -> std::same_as<bool>;
            {       bit_reference == const_bit_reference } -> std::same_as<bool>;
            {       bit_reference <  const_bit_reference } -> std::same_as<bool>;
        };

        template <typename T>
        concept Referenceable = requires(T type)
        {
            requires std::is_reference_v<std::add_lvalue_reference_t<T>>;
        };

        template <typename Iterator>
        concept LegacyIterator = requires(Iterator i)
        {
            {   *i } -> Referenceable;
            {  ++i } -> std::same_as<Iterator&>;
            { *i++ } -> Referenceable;
        } && std::copyable<Iterator>;

        template <typename Iterator>
        concept LegacyInputIterator = LegacyIterator<Iterator>
            && std::equality_comparable<Iterator>
            && requires(Iterator i)
        {
            typename std::incrementable_traits<Iterator>::difference_type;
            typename std::indirectly_readable_traits<Iterator>::value_type;
            typename std::common_reference_t<std::iter_reference_t<Iterator>&&,
                                            typename std::indirectly_readable_traits<Iterator>::value_type&>;
            *i++;
            typename std::common_reference_t<decltype(*i++)&&,
                                            typename std::indirectly_readable_traits<Iterator>::value_type&>;
            requires std::signed_integral<typename std::incrementable_traits<Iterator>::difference_type>;
        };

        template <typename Iterator, typename IsBitPacked>
        concept LegacyForwardIterator = LegacyInputIterator<Iterator>
            && std::constructible_from<Iterator>
            && (
                (
                    std::is_lvalue_reference_v<std::iter_reference_t<Iterator>>
                    && std::same_as<std::remove_cvref_t<std::iter_reference_t<Iterator>>,
                                    typename std::indirectly_readable_traits<Iterator>::value_type>
                )
                || (
                    std::same_as<IsBitPacked, typename std::true_type>
                    && impl::BitReference<typename std::iter_reference_t<Iterator>>
                )
            )
            && requires(Iterator i)
        {
            {  i++ } -> std::convertible_to<const Iterator&>;
            { *i++ } -> std::same_as<std::iter_reference_t<Iterator>>;
        };

        template<class Iterator, typename IsBitPacked>
        concept LegacyBidirectionalIterator = LegacyForwardIterator<Iterator, IsBitPacked>
            && requires(Iterator i)
        {
            {  --i } -> std::same_as<Iterator&>;
            {  i-- } -> std::convertible_to<const Iterator&>;
            { *i-- } -> std::same_as<std::iter_reference_t<Iterator>>;
        };

        template <typename Iterator, typename IsBitPacked>
        concept LegacyRandomAccessIterator = LegacyBidirectionalIterator<Iterator, IsBitPacked>
            && std::totally_ordered<Iterator>
            && requires(Iterator i, typename std::incrementable_traits<Iterator>::difference_type n)
        {
            { i += n } -> std::same_as<Iterator&>;
            { i -= n } -> std::same_as<Iterator&>;
            { i +  n } -> std::same_as<Iterator>;
            { n +  i } -> std::same_as<Iterator>;
            { i -  n } -> std::same_as<Iterator>;
            { i -  i } -> std::same_as<decltype(n)>;
            {  i[n]  } -> std::convertible_to<std::iter_reference_t<Iterator>>;
        };
    }

    /**
     * @brief Defines a container that satisfies Container semantics.
     *
     * @tparam ContainerType The tested container type.
     */
    template <typename ContainerType> 
    concept Container = requires(ContainerType a, const ContainerType b) 
    {
        typename ContainerType::value_type;
        typename ContainerType::reference;
        typename ContainerType::const_reference;
        typename ContainerType::iterator;
        typename ContainerType::const_iterator;
        typename ContainerType::difference_type;
        typename ContainerType::size_type;
        requires std::regular<ContainerType>;
        requires std::swappable<ContainerType>;
        requires std::destructible<typename ContainerType::value_type>;
        requires std::same_as<typename ContainerType::reference, typename ContainerType::value_type&>
            || (
                std::same_as<typename ContainerType::value_type, bool>
                && impl::BitReference<typename ContainerType::reference>
            );
        requires std::same_as<typename ContainerType::const_reference, const typename ContainerType::value_type&>
            || (
                std::same_as<typename ContainerType::value_type, bool>
                && std::same_as<typename ContainerType::const_reference, bool>
            );
        requires std::forward_iterator<typename ContainerType::iterator>;
        requires std::forward_iterator<typename ContainerType::const_iterator>;
        requires std::signed_integral<typename ContainerType::difference_type>;
        requires std::same_as<typename ContainerType::difference_type,
            typename std::iterator_traits<typename ContainerType::iterator>::difference_type>;
        requires std::same_as<typename ContainerType::difference_type,
            typename std::iterator_traits<typename ContainerType::const_iterator>::difference_type>;
        { a.begin()    } -> std::same_as<typename ContainerType::iterator>;
        { a.end()      } -> std::same_as<typename ContainerType::iterator>;
        { b.begin()    } -> std::same_as<typename ContainerType::const_iterator>;
        { b.end()      } -> std::same_as<typename ContainerType::const_iterator>;
        { a.cbegin()   } -> std::same_as<typename ContainerType::const_iterator>;
        { a.cend()     } -> std::same_as<typename ContainerType::const_iterator>;
        { a.size()     } -> std::same_as<typename ContainerType::size_type>;
        { a.max_size() } -> std::same_as<typename ContainerType::size_type>;
        { a.empty()    } -> std::same_as<bool>;
    };

    /**
     * @brief Defines a container whose capcity may change.
     *
     * @tparam ContainerType The tested container type.
     */
    template <typename ContainerType>
    concept ResizableContainer = Container<ContainerType>
        && requires(ContainerType c, size_t n)
    {
        { c.resize(n) } -> std::same_as<void>;
    };

    /**
     * @brief Defines a container whose capcity is fixed.
     *
     * @tparam ContainerType The tested container type.
     */
    template <typename ContainerType>
    concept FixedSizeContainer = Container<ContainerType>
        && requires(ContainerType c)
    {
        { std::tuple_size_v<ContainerType> } -> std::same_as<size_t>;
    };

    /**
     * @brief Defines a container that stores objects in contiguous memory locations.
     *
     * Std Lib Contiguous Containers:
     *   * basic_string
     *   * array
     *   * vector
     *
     * @tparam ContainerType The tested container type.
     */
    template <typename ContainerType>
    concept ContiguousContainer = Container<ContainerType>
        && impl::LegacyRandomAccessIterator<typename ContainerType::iterator,
                                                std::conditional_t<
                                                    std::is_same_v<typename ContainerType::value_type, bool>,
                                                    typename std::true_type,
                                                    typename std::false_type
                                                >
                                            >
        && impl::LegacyRandomAccessIterator<typename ContainerType::const_iterator,
                                                std::conditional_t<
                                                    std::is_same_v<typename ContainerType::value_type, bool>,
                                                    typename std::true_type,
                                                    typename std::false_type
                                                >
                                            >
        && (
            // TODO(Matthew): What can be used to check here? _Bit_iterator returns newly constructed _Bit_reference on
            //                dereference & _Bit_reference only returns bools by copy.
            std::same_as<typename ContainerType::value_type, bool>
            || (
                std::contiguous_iterator<typename ContainerType::iterator>
                && std::contiguous_iterator<typename ContainerType::const_iterator>
            )
        );

    /**
     * @brief Defines a container whose capcity may change.
     *
     * @tparam ContainerType The tested container type.
     */
    template <typename ContainerType>
    concept ResizableContiguousContainer = ContiguousContainer<ContainerType>
        && ResizableContainer<ContainerType>;

    /**
     * @brief Defines a container whose capcity is fixed.
     *
     * @tparam ContainerType The tested container type.
     */
    template <typename ContainerType>
    concept FixedSizeContiguousContainer = ContiguousContainer<ContainerType>
        && FixedSizeContainer<ContainerType>;

    /**
     * @brief Defines a container that stores key-value pairs.
     *
     * Std Lib Map Containers:
     *   * map
     *   * multimap
     *   * unordered_map
     *   * unordered_multimap
     *
     * @tparam ContainerType The tested container type.
     */
    template <typename ContainerType>
    concept MapContainer = Container<ContainerType>
        && requires(ContainerType c)
    {
        typename ContainerType::key_type;
        typename ContainerType::hasher;
        typename ContainerType::key_equal;
    }
        && requires(
            ContainerType                     c,
            ContainerType::key_type         key,
            ContainerType::mapped_type    value,
            ContainerType::iterator          it,
            ContainerType::mapped_type const_it
        )
    {
        { c()[key]    } -> std::same_as<typename ContainerType::mapped_type&>;
        { c().at(key) } -> std::same_as<typename ContainerType::mapped_type&>;
        { c().insert({key, value}) } -> std::same_as<std::pair<typename ContainerType::iterator, bool>>;
        { c().emplace()            } -> std::same_as<std::pair<typename ContainerType::iterator, bool>>;
        { c().erase(it)                 } -> std::same_as<typename ContainerType::iterator>;
        { c().erase(const_it)           } -> std::same_as<typename ContainerType::iterator>;
        { c().erase(const_it, const_it) } -> std::same_as<typename ContainerType::iterator>;
        { c().swap(c()) };
    };
}

#endif // __hemlock_basic_concepts_hpp

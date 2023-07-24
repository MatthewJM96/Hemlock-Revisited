#ifndef __hemlock_io_yaml_converters_unordered_map_hpp
#define __hemlock_io_yaml_converters_unordered_map_hpp

#include "io/filesystem.hpp"

namespace YAML {
    template <
        typename KeyType,
        typename ValueType,
        typename Hash,
        typename KeyEqual,
        typename Allocator>
    struct convert<std::unordered_map<KeyType, ValueType, Hash, KeyEqual, Allocator>> {
        static Node encode(
            const std::unordered_map<KeyType, ValueType, Hash, KeyEqual, Allocator>& rhs
        ) {
            Node node(NodeType::Map);
            for (const auto& element : rhs)
                node.force_insert(element.first, element.second);
            return node;
        }

        static bool decode(
            const Node&                                                        node,
            std::unordered_map<KeyType, ValueType, Hash, KeyEqual, Allocator>& rhs
        ) {
            if (!node.IsMap()) return false;

            rhs.clear();

            for (const auto& element : node)
                rhs[element.first.as<KeyType>()] = element.second.as<ValueType>();

            return true;
        }
    };
}  // namespace YAML

#endif  // __hemlock_io_yaml_converters_unordered_map_hpp

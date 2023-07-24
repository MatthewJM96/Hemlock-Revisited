#ifndef __hemlock_io_yaml_converters_optional_hpp
#define __hemlock_io_yaml_converters_optional_hpp

namespace YAML {
    template <typename Type>
    struct convert<std::optional<Type>> {
        static Node encode(const std::optional<Type>& opt) {
            if (!opt.has_value()) return Node{};

            return convert<Type>::encode(opt.value());
        }

        static bool decode(const Node& node, std::optional<Type>& opt) {
            if (!node.IsDefined() || node.IsNull()) return false;

            Type tmp;
            bool success = convert<Type>::decode(node, tmp);

            if (!success) return false;

            opt = tmp;
            return true;
        }
    };
}  // namespace YAML

#endif  // __hemlock_io_yaml_converters_optional_hpp

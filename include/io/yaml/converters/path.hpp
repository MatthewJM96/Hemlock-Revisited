#ifndef __hemlock_io_yaml_converters_path_hpp
#define __hemlock_io_yaml_converters_path_hpp

#include "io/filesystem.hpp"

namespace YAML {
    template <>
    struct convert<hio::fs::path> {
        static Node encode(const hio::fs::path& path) { return Node{ path.string() }; }

        static bool decode(const Node& node, hio::fs::path& result) {
            if (!node.IsScalar()) {
                return false;
            }

            result = hio::fs::path{ node.as<std::string>() };

            return true;
        }
    };
}  // namespace YAML

#endif  // __hemlock_io_yaml_converters_path_hpp

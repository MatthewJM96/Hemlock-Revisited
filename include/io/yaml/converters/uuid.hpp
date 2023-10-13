#ifndef __hemlock_io_yaml_converters_uuid_hpp
#define __hemlock_io_yaml_converters_uuid_hpp

#include "io/filesystem.hpp"

namespace YAML {
    template <>
    struct convert<hemlock::UUID> {
        static Node encode(const hemlock::UUID& uuid) {
            std::stringstream ss;
            ss << uuid;

            return Node{ ss.str() };
        }

        static bool decode(const Node& node, hemlock::UUID& result) {
            if (!node.IsScalar()) {
                return false;
            }

            std::stringstream ss;

            ss << node.as<std::string>();
            ss >> result;

            return true;
        }
    };
}  // namespace YAML

#endif  // __hemlock_io_yaml_converters_uuid_hpp

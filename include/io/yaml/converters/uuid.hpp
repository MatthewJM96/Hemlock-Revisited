#ifndef __hemlock_io_yaml_converters_uuid_hpp
#define __hemlock_io_yaml_converters_uuid_hpp

#include "io/filesystem.hpp"

namespace YAML {
    template <>
    struct convert<boost::uuids::uuid> {
        static Node encode(const boost::uuids::uuid& uuid) {
            std::stringstream ss;
            ss << uuid;

            return Node{ ss.str() };
        }

        static bool decode(const Node& node, boost::uuids::uuid& result) {
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

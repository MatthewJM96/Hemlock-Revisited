#ifndef __hemlock_io_yaml_converters_semver_hpp
#define __hemlock_io_yaml_converters_semver_hpp

#include "version/semver.h"

namespace YAML {
    template <>
    struct convert<hemlock::SemanticVersion> {
        static Node encode(const hemlock::SemanticVersion& semver) {
            return Node{ semver.string() };
        }

        static bool decode(const Node& node, hemlock::SemanticVersion& semver) {
            if (!node.IsScalar()) {
                return false;
            }

            return semver.load(node.as<std::string>());
        }
    };
}  // namespace YAML

#endif  // __hemlock_io_yaml_converters_semver_hpp

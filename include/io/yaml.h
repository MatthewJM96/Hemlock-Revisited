#ifndef __hemlock_yaml_hpp
#define __hemlock_yaml_hpp

namespace hemlock {
    namespace io {
        namespace yaml {
            YAML::Node merge(YAML::Node left, YAML::Node right);
        }
    }
}
namespace hio = hemlock::io;

#endif // __hemlock_yaml_hpp

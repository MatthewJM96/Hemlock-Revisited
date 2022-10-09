#ifndef __hemlock_io_yaml_h
#define __hemlock_io_yaml_h

namespace hemlock {
    namespace io {
        namespace yaml {
            /**
             * @brief Merge the right-hand YAML document into the
             * left-hand document. Neither is modified in this
             * operation, the returned document is a new document.
             *
             * @param left The left-hand YAML document.
             * @param right The right-hand YAML document.
             * @return YAML::Node The resulting YAML document.
             */ 
            YAML::Node merge(const YAML::Node left, const YAML::Node right);
        }
    }
}
namespace hio = hemlock::io;

#endif // __hemlock_io_yaml_h

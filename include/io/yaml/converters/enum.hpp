#ifndef __hemlock_io_yaml_converters_enum_hpp
#define __hemlock_io_yaml_converters_enum_hpp

#include "io/serialisation.hpp"

namespace YAML {

    /*************\
     * Enum Type *
    \*************/

    template <typename Type>
        requires ( std::is_enum<Type>::value )
    struct convert<Type> {
        using UnderlyingType = typename std::underlying_type<Type>::type;

        static Node encode(const Type& val) {
            return Node(std::string(hio::serialisable_enum_name(val)));
        }

        static bool decode(const Node& node, Type& result) {
            if(!node.IsScalar()) {
                return false;
            }

            result = hio::serialisable_enum_val<Type>(node.as<std::string>());

            return true;
        }
    };

}

#endif // __hemlock_io_yaml_converters_enum_hpp

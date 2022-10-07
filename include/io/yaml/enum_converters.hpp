#ifndef __hemlock_yaml_enum_converters_hpp
#define __hemlock_yaml_enum_converters_hpp

// TODO(Matthew): Finish impl, look at mesh.h for inspiration, as well as https://stackoverflow.com/questions/11714325/how-to-get-enum-item-name-from-its-value
//                  Question: how to know to access NAME_Names not SOME_OTHER_NAME_Names for a given Type?

#if !defined(H_DEF_SERIALISABLE_ENUM_CLASS)
#define H_DEF_SERIALISABLE_ENUM_CLASS(NAME) \
enum class NAME {                           \g
    STUFF THAT ENUMERATES                   \
};                                          \
                                            \
const char* NAME_Names[] = {                \
    STUFF THAT NAMES ENUMERATIONS           \
};
#endif // !defined(H_DEF_SERIALISABLE_ENUM_CLASS)

namespace YAML {

    /*************\
     * Enum Type *
    \*************/

    template <typename Type>
        requires ( std::is_enum<Type>::value )
    struct convert<Type> {
        using UnderlyingType = typename std::underlying_type<Type>::type;

        static Node encode(const Type& val) {
            return Node(static_cast<UnderlyingType>(val));
        }

        static bool decode(const Node& node, Type& result) {
            if(!node.IsScalar()) {
                return false;
            }

            result = static_cast<Type>(node.as<UnderlyingType>());

            return true;
        }
    };

}

#endif // __hemlock_yaml_enum_converters_hpp

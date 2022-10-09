#ifndef __hemlock_io_yaml_converters_struct_hpp
#define __hemlock_io_yaml_converters_struct_hpp

#include "io/serialisation.hpp"

#if !defined(H_DEF_STRUCT_YAML_CONVERSION)
#define H_STRUCT_ENTRY_NAME(NAME, TYPE) #NAME
#define H_STRUCT_ENTRY_FIELD(NAME, TYPE) NAME
#define H_STRUCT_ENTRY_TYPE(NAME, TYPE) TYPE

#if !defined(H_STRUCT_ENTRY_ENCODE)
#define H_STRUCT_ENTRY_ENCODE(ENTRY_INFO)               \
node[H_STRUCT_ENTRY_NAME ENTRY_INFO] = YAML::Node(      \
    val.H_STRUCT_ENTRY_FIELD ENTRY_INFO                 \
)
#endif //!defined(H_STRUCT_ENTRY_ENCODE)

#if !defined(H_STRUCT_ENTRY_DECODE)
#define H_STRUCT_ENTRY_DECODE(ENTRY_INFO)               \
if (                                                    \
    !convert<H_STRUCT_ENTRY_TYPE ENTRY_INFO>::decode(   \
        node[H_STRUCT_ENTRY_NAME ENTRY_INFO],           \
        result.H_STRUCT_ENTRY_FIELD ENTRY_INFO          \
    )                                                   \
) {                                                     \
    return false;                                       \
}
#endif //!defined(H_STRUCT_ENTRY_DECODE)

#define H_DEF_STRUCT_YAML_CONVERSION(TYPE, ...)                     \
namespace YAML {                                                    \
    template <>                                                     \
    struct convert<TYPE> {                                          \
        static Node encode(const TYPE& val) {                       \
            YAML::Node node(YAML::NodeType::Map);                   \
                                                                    \
            MAP(H_STRUCT_ENTRY_ENCODE, SEMICOLON, __VA_ARGS__);     \
                                                                    \
            return node;                                            \
        }                                                           \
                                                                    \
        static bool decode(const Node& node, TYPE& result) {        \
            MAP(H_STRUCT_ENTRY_DECODE, SEMICOLON, __VA_ARGS__);     \
                                                                    \
            return true;                                            \
        }                                                           \
    };                                                              \
}
#endif //!defined(H_DEF_STRUCT_YAML_CONVERSION)

#endif // __hemlock_io_yaml_converters_struct_hpp

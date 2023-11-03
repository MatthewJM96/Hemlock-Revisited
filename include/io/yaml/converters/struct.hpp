#ifndef __hemlock_io_yaml_converters_struct_hpp
#define __hemlock_io_yaml_converters_struct_hpp

/*************************************\
 * Generic helpers for field access. *
\*************************************/

#if !defined(H_FIELD_NAME)
#  define H_FIELD_NAME(NAME, TYPE) NAME
#endif  //! defined(H_FIELD_NAME)
#if !defined(H_FIELD_NAME_STR)
#  define H_FIELD_NAME_STR(NAME, TYPE) #NAME
#endif  //! defined(H_FIELD_NAME_STR)
#if !defined(H_FIELD_TYPE)
#  define H_FIELD_TYPE(NAME, TYPE) TYPE
#endif  //! defined(H_FIELD_TYPE)

#if !defined(H_DECL_YAML_CONVERSION_OF_STRUCT)
#  define H_DECL_YAML_CONVERSION_OF_STRUCT(NAMESPACE, NAME)                            \
    namespace YAML {                                                                   \
      template <>                                                                      \
      struct convert<NAMESPACE::NAME> {                                                \
        static Node encode(const NAMESPACE::NAME& val);                                \
                                                                                       \
        static bool decode(const Node& node, NAMESPACE::NAME& result);                 \
      };                                                                               \
    }
#endif  //! defined(H_DECL_YAML_CONVERSION_OF_STRUCT)

#if !defined(H_DEF_YAML_CONVERSION_OF_STRUCT)

#  if !defined(H_WRITE_STRUCT_ENTRY_ENCODE)
#    define H_WRITE_STRUCT_ENTRY_ENCODE(FIELD_DATA)                                    \
      node[H_FIELD_NAME_STR FIELD_DATA] = YAML::Node(val.H_FIELD_NAME FIELD_DATA)
#  endif  //! defined(H_WRITE_STRUCT_ENTRY_ENCODE)

#  if !defined(H_WRITE_STRUCT_ENTRY_DECODE)
#    define H_WRITE_STRUCT_ENTRY_DECODE(FIELD_DATA)                                    \
      if (!convert<H_FIELD_TYPE FIELD_DATA>::decode(                                   \
              node[H_FIELD_NAME_STR FIELD_DATA], result.H_FIELD_NAME FIELD_DATA        \
          ))                                                                           \
      {                                                                                \
        return false;                                                                  \
      }
#  endif  //! defined(H_WRITE_STRUCT_ENTRY_DECODE)

#  define H_DEF_YAML_CONVERSION_OF_STRUCT(NAMESPACE, NAME, ...)                        \
    [[maybe_unused]] YAML::Node YAML::convert<NAMESPACE::NAME>::encode(                \
        const NAMESPACE::NAME& val                                                     \
    ) {                                                                                \
      YAML::Node node(YAML::NodeType::Map);                                            \
                                                                                       \
      MAP(H_WRITE_STRUCT_ENTRY_ENCODE, SEMICOLON, __VA_ARGS__);                        \
                                                                                       \
      return node;                                                                     \
    }                                                                                  \
                                                                                       \
    [[maybe_unused]] bool YAML::convert<NAMESPACE::NAME>::decode(                      \
        const Node& node, NAMESPACE::NAME& result                                      \
    ) {                                                                                \
      MAP(H_WRITE_STRUCT_ENTRY_DECODE, SEMICOLON, __VA_ARGS__);                        \
                                                                                       \
      return true;                                                                     \
    }
#endif  //! defined(H_DEF_YAML_CONVERSION_OF_STRUCT)

#endif  // __hemlock_io_yaml_converters_struct_hpp

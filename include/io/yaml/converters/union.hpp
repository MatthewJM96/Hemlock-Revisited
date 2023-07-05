#ifndef __hemlock_io_yaml_converters_union_hpp
#define __hemlock_io_yaml_converters_union_hpp

// TODO(Matthew): Fix defining enum we want serialisable without using enum
//                serialisation macros.

#if !defined(H_POD_STRUCT)
#  define H_POD_STRUCT 1
#endif  // !defined(H_POD_STRUCT)

#if !defined(H_NON_POD_TYPE)
#  define H_NON_POD_TYPE 0
#endif  // !defined(H_NON_POD_TYPE)

#if !defined(H_FIELD_NAME)
#  define H_FIELD_NAME(NAME, TYPE) NAME
#endif  //! defined(H_FIELD_NAME)
#if !defined(H_FIELD_NAME_STR)
#  define H_FIELD_NAME_STR(NAME, TYPE) #NAME
#endif  //! defined(H_FIELD_NAME_STR)
#if !defined(H_FIELD_TYPE)
#  define H_FIELD_TYPE(NAME, TYPE) TYPE
#endif  //! defined(H_FIELD_TYPE)

#if !defined(H_UNION_ENTRY_NAME)
#  define H_UNION_ENTRY_NAME(IS_POD_STRUCT, NAME, ...) NAME
#endif  //! defined(H_UNION_ENTRY_NAME)

#if !defined(H_INDIRECT_UNION_ENTRY_NAME)
#  define H_INDIRECT_UNION_ENTRY_NAME(ARGS) H_UNION_ENTRY_NAME ARGS
#endif  //! defined(H_INDIRECT_UNION_ENTRY_NAME)

#if !defined(H_DEF_YAML_CONVERSION_OF_UNION)

#  if !defined(H_WRITE_UNION_FIELD_ENCODE)
#    define H_WRITE_UNION_FIELD_ENCODE(FIELD_DATA)                                     \
      node[H_FIELD_NAME_STR FIELD_DATA] = YAML::Node(val.H_FIELD_NAME FIELD_DATA);
#  endif  //! defined(H_WRITE_UNION_FIELD_ENCODE)

#  if !defined(H_WRITE_UNION_FIELD_DECODE)
#    define H_WRITE_UNION_FIELD_DECODE(FIELD_DATA)                                     \
      if (!convert<H_FIELD_TYPE FIELD_DATA>::decode(                                   \
              node[H_FIELD_NAME_STR FIELD_DATA], result.H_FIELD_NAME FIELD_DATA        \
          ))                                                                           \
      {                                                                                \
        return false;                                                                  \
      }
#  endif  //! defined(H_WRITE_UNION_FIELD_DECODE)

#  define H_WRITE_UNION_ENCODE(IS_POD_STRUCT, NAME, ...)                               \
NAME:                                                                                  \
{                                                                                      \
MAP(H_WRITE_UNION_FIELD_ENCODE, EMPTY, __VA_ARGS__)                                    \
break;                                                                                 \
}

#  define H_WRITE_UNION_DECODE(IS_POD_STRUCT, NAME, ...)                               \
NAME:                                                                                  \
{                                                                                      \
MAP(H_WRITE_UNION_FIELD_DECODE, EMPTY, __VA_ARGS__)                                    \
break;                                                                                 \
}

#  define H_INDIRECT_WRITE_UNION_ENCODE(KIND_TYPE, ARGS)                               \
    case KIND_TYPE ::H_WRITE_UNION_ENCODE ARGS
#  define H_INDIRECT_WRITE_UNION_DECODE(KIND_TYPE, ARGS)                               \
    case KIND_TYPE ::H_WRITE_UNION_DECODE ARGS

#  define H_DEF_YAML_CONVERSION_OF_UNION(UNION_TYPE, KIND_UNDERLYING_TYPE, ...)        \
    namespace YAML {                                                                   \
      template <>                                                                      \
      struct convert<UNION_TYPE> {                                                     \
        static Node encode(const UNION_TYPE& val) {                                    \
          YAML::Node node(YAML::NodeType::Map);                                        \
                                                                                       \
          node["kind"] = YAML::Node(val.kind);                                         \
                                                                                       \
          switch (val.kind) {                                                          \
            default:                                                                   \
            {                                                                          \
              debug_printf(                                                            \
                  "Could not encode MyUnion with kind %i",                             \
                  static_cast<KIND_UNDERLYING_TYPE>(val.kind)                          \
              );                                                                       \
              return YAML::Node();                                                     \
            }                                                                          \
              BIND_MAP_2(                                                              \
                  H_INDIRECT_WRITE_UNION_ENCODE,                                       \
                  CAT(UNION_TYPE, Kind),                                               \
                  EMPTY,                                                               \
                  __VA_ARGS__                                                          \
              )                                                                        \
          }                                                                            \
                                                                                       \
          return node;                                                                 \
        }                                                                              \
                                                                                       \
        static bool decode(const Node& node, UNION_TYPE& result) {                     \
          if (!convert<UNION_TYPE##Kind>::decode(node["kind"], result.kind)) {         \
            return false;                                                              \
          }                                                                            \
                                                                                       \
          switch (result.kind) {                                                       \
            default:                                                                   \
            {                                                                          \
              debug_printf(                                                            \
                  "Could not encode MyUnion with kind %i",                             \
                  static_cast<KIND_UNDERLYING_TYPE>(result.kind)                       \
              );                                                                       \
              return false;                                                            \
            }                                                                          \
              BIND_MAP_2(                                                              \
                  H_INDIRECT_WRITE_UNION_DECODE,                                       \
                  CAT(UNION_TYPE, Kind),                                               \
                  EMPTY,                                                               \
                  __VA_ARGS__                                                          \
              )                                                                        \
          }                                                                            \
                                                                                       \
          return true;                                                                 \
        }                                                                              \
      };                                                                               \
    }
#endif  //! defined(H_DEF_YAML_CONVERSION_OF_UNION)

#if !defined(H_DEF_UNION_WITH_YAML_CONVERSION)
#  define H_UNION_ENTRY_TO_FIELD(FIELD_DATA)                                           \
    H_FIELD_TYPE FIELD_DATA H_FIELD_NAME FIELD_DATA
#  define H_WRITE_UNION_KIND(ELEM) H_UNION_ENTRY_NAME ELEM,

#  define H_WRITE_UNION_POD_STRUCT_ENTRY(...)                                          \
    struct {                                                                           \
      MAP(H_UNION_ENTRY_TO_FIELD, SEMICOLON, __VA_ARGS__);                             \
    };
#  define H_WRITE_UNION_NON_POD_TYPE_ENTRY(FIELD_DATA)                                 \
    H_UNION_ENTRY_TO_FIELD(FIELD_DATA);

#  define H_WRITE_UNION_ENTRY(IS_POD_STRUCT, NAME, ...)                                \
    IF_ELSE(BOOL(IS_POD_STRUCT))                                                       \
    (H_WRITE_UNION_POD_STRUCT_ENTRY(__VA_ARGS__),                                      \
     H_WRITE_UNION_NON_POD_TYPE_ENTRY(__VA_ARGS__))

#  define H_INDIRECT_WRITE_UNION_ENTRY(ARGS) H_WRITE_UNION_ENTRY ARGS

#  define H_DEF_UNION_WITH_YAML_CONVERSION(NAMESPACE, NAME, KIND_TYPE, ...)            \
    namespace NAMESPACE {                                                              \
      enum class NAME##Kind : KIND_TYPE{ MAP(H_WRITE_UNION_KIND, EMPTY, __VA_ARGS__)   \
                                             SENTINEL };                               \
      struct NAME {                                                                    \
        NAME##Kind kind;                                                               \
        union {                                                                        \
          MAP_2(H_INDIRECT_WRITE_UNION_ENTRY, EMPTY, __VA_ARGS__)                      \
        };                                                                             \
      };                                                                               \
    }                                                                                  \
    H_DECL_SERIALISATION_OF_ENUM(                                                      \
        NAMESPACE, NAME##Kind, MAP(H_INDIRECT_UNION_ENTRY_NAME, COMMA, __VA_ARGS__)    \
    )                                                                                  \
    H_DEF_SERIALISATION_OF_ENUM(NAMESPACE, NAME##Kind)                                 \
    H_DEF_YAML_CONVERSION_OF_UNION(NAMESPACE ::NAME, KIND_TYPE, __VA_ARGS__)
#endif  //! defined(H_DEF_UNION_WITH_YAML_CONVERSION)

#endif  // __hemlock_io_yaml_converters_union_hpp

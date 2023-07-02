#ifndef __hemlock_io_yaml_converters_union_hpp
#define __hemlock_io_yaml_converters_union_hpp

// TODO(Matthew): Fix defining enum we want serialisable without using enum
//                serialisation macros.

#if !defined(H_FIELD_NAME)
#  define H_FIELD_NAME(NAME, TYPE) NAME
#endif  //! defined(H_FIELD_NAME)
#if !defined(H_FIELD_NAME_STR)
#  define H_FIELD_NAME_STR(NAME, TYPE) #NAME
#endif  //! defined(H_FIELD_NAME_STR)
#if !defined(H_FIELD_TYPE)
#  define H_FIELD_TYPE(NAME, TYPE) TYPE
#endif  //! defined(H_FIELD_TYPE)

#if !defined(H_UNION_ELEM_VALUE)
#  define H_UNION_ELEM_VALUE(_, VALUE, TYPE) VALUE
#endif  //! defined(H_UNION_ELEM_VALUE)
#if !defined(H_UNION_ELEM_TYPE)
#  define H_UNION_ELEM_TYPE(_, VALUE, TYPE) TYPE
#endif  //! defined(H_UNION_ELEM_TYPE)

#define H_UNION_ENTRY()         1
#define H_NEXT_UNION_KIND()     1
#define H_CONTINUE_UNION_KIND() 0

#if !defined(H_DEF_YAML_CONVERSION_OF_UNION)
#  define H_SWITCH_UNION_CASE(KIND_TYPE, KIND)                                         \
    break;                                                                             \
    }                                                                                  \
    case KIND_TYPE ::KIND:                                                             \
    {
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

#  define H_WRITE_UNION_ENCODE(KIND_TYPE, ELEM)                                        \
    IF_ELSE(BOOL(H_UNION_ELEM_TYPE ELEM))                                              \
    (DEFER4(H_SWITCH_UNION_CASE)(KIND_TYPE, H_UNION_ELEM_VALUE ELEM),                  \
     DEFER4(H_WRITE_UNION_FIELD_ENCODE)(H_UNION_ELEM_VALUE ELEM))
#  define H_WRITE_UNION_DECODE(KIND_TYPE, ELEM)                                        \
    IF_ELSE(BOOL(H_UNION_ELEM_TYPE ELEM))                                              \
    (DEFER4(H_SWITCH_UNION_CASE)(KIND_TYPE, H_UNION_ELEM_VALUE ELEM),                  \
     DEFER4(H_WRITE_UNION_FIELD_DECODE)(H_UNION_ELEM_VALUE ELEM))

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
              BIND_MAP(                                                                \
                  H_WRITE_UNION_ENCODE, CAT(UNION_TYPE, Kind), EMPTY, __VA_ARGS__      \
              )                                                                        \
            }                                                                          \
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
                  static_cast<KIND_UNDERLYING_TYPE>(val.kind)                          \
              );                                                                       \
              return false;                                                            \
              BIND_MAP(                                                                \
                  H_WRITE_UNION_DECODE, CAT(UNION_TYPE, Kind), EMPTY, __VA_ARGS__      \
              )                                                                        \
            }                                                                          \
          }                                                                            \
                                                                                       \
          return true;                                                                 \
        }                                                                              \
      };                                                                               \
    }
#endif  //! defined(H_DEF_YAML_CONVERSION_OF_UNION)

#if !defined(H_DEF_UNION_WITH_YAML_CONVERSION)
#  define H_SWITCH_UNION_STRUCT()                                                      \
    }                                                                                  \
    ;                                                                                  \
    struct {
#  define H_WRITE_UNION_FIELD(FIELD_DATA)                                              \
    H_FIELD_TYPE FIELD_DATA H_FIELD_NAME FIELD_DATA

#  define H_WRITE_UNION_KIND(ELEM)                                                     \
    IF(BOOL(H_UNION_ELEM_TYPE ELEM)) (H_UNION_ELEM_VALUE ELEM, )

#  define H_WRITE_UNION_STRUCT_ELEMENT(ELEM)                                           \
    IF_ELSE(BOOL(H_UNION_ELEM_TYPE ELEM))                                              \
    (DEFER4(H_SWITCH_UNION_STRUCT)(),                                                  \
     DEFER4(H_WRITE_UNION_FIELD)(H_UNION_ELEM_VALUE ELEM);)

#  define H_DEF_UNION_WITH_YAML_CONVERSION(NAMESPACE, NAME, KIND_TYPE, ...)            \
    namespace NAMESPACE {                                                              \
      enum class NAME##Kind : KIND_TYPE{ MAP(H_WRITE_UNION_KIND, EMPTY, __VA_ARGS__)   \
                                             SENTINEL };                               \
      struct NAME {                                                                    \
        NAME##Kind kind;                                                               \
        union {                                                                        \
          struct {                                                                     \
            MAP(H_WRITE_UNION_STRUCT_ELEMENT, EMPTY, __VA_ARGS__)                      \
          };                                                                           \
        };                                                                             \
      };                                                                               \
    }                                                                                  \
    H_DEF_YAML_CONVERSION_OF_UNION(NAMESPACE ::NAME, KIND_TYPE, __VA_ARGS__)
#endif  //! defined(H_DEF_UNION_WITH_YAML_CONVERSION)

#endif  // __hemlock_io_yaml_converters_union_hpp

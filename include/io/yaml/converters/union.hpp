#ifndef __hemlock_io_yaml_converters_union_hpp
#define __hemlock_io_yaml_converters_union_hpp

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

#if !defined(H_DECL_YAML_CONVERSION_OF_UNION)

/*******************************************\
 * Union YAML conversion decl entry macro. *
\*******************************************/

#  define H_DECL_YAML_CONVERSION_OF_UNION(UNION_TYPE)                                  \
    namespace YAML {                                                                   \
      template <>                                                                      \
      struct convert<UNION_TYPE> {                                                     \
        static Node encode(const UNION_TYPE& val);                                     \
                                                                                       \
        static bool decode(const Node& node, UNION_TYPE& result);                      \
      };                                                                               \
    }
#endif  //! defined(H_DECL_YAML_CONVERSION_OF_UNION)

#if !defined(H_DEF_YAML_CONVERSION_OF_UNION)

/***************************************\
 * Union YAML conversion def helpers. *
\***************************************/

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

#  define H_WRITE_UNION_ENCODE(NAME, IS_POD_STRUCT, ...)                               \
NAME:                                                                                  \
{                                                                                      \
MAP(H_WRITE_UNION_FIELD_ENCODE, EMPTY, __VA_ARGS__)                                    \
break;                                                                                 \
}

#  define H_WRITE_UNION_DECODE(NAME, IS_POD_STRUCT, ...)                               \
NAME:                                                                                  \
{                                                                                      \
MAP(H_WRITE_UNION_FIELD_DECODE, EMPTY, __VA_ARGS__)                                    \
break;                                                                                 \
}

#  define H_INDIRECT_WRITE_UNION_ENCODE(KIND_TYPE, ARGS)                               \
    case KIND_TYPE ::H_WRITE_UNION_ENCODE ARGS
#  define H_INDIRECT_WRITE_UNION_DECODE(KIND_TYPE, ARGS)                               \
    case KIND_TYPE ::H_WRITE_UNION_DECODE ARGS

/*******************************************\
 * Union YAML conversion def entry macro. *
\*******************************************/

#  define H_DEF_YAML_CONVERSION_OF_UNION(UNION_TYPE, KIND_UNDERLYING_TYPE, ...)        \
    [[maybe_unused]] YAML::Node YAML::convert<UNION_TYPE>::encode(                     \
        const UNION_TYPE& val                                                          \
    ) {                                                                                \
      YAML::Node node(YAML::NodeType::Map);                                            \
                                                                                       \
      node["kind"] = YAML::Node(val.kind);                                             \
                                                                                       \
      switch (val.kind) {                                                              \
        default:                                                                       \
        {                                                                              \
          debug_printf(                                                                \
              "Could not encode MyUnion with kind %i",                                 \
              static_cast<KIND_UNDERLYING_TYPE>(val.kind)                              \
          );                                                                           \
          return YAML::Node();                                                         \
        }                                                                              \
          BIND_MAP_2(                                                                  \
              H_INDIRECT_WRITE_UNION_ENCODE, CAT(UNION_TYPE, Kind), EMPTY, __VA_ARGS__ \
          )                                                                            \
      }                                                                                \
                                                                                       \
      return node;                                                                     \
    }                                                                                  \
                                                                                       \
    [[maybe_unused]] bool YAML::convert<UNION_TYPE>::decode(                           \
        const Node& node, UNION_TYPE& result                                           \
    ) {                                                                                \
      if (!convert<UNION_TYPE##Kind>::decode(node["kind"], result.kind)) {             \
        return false;                                                                  \
      }                                                                                \
                                                                                       \
      switch (result.kind) {                                                           \
        default:                                                                       \
        {                                                                              \
          debug_printf(                                                                \
              "Could not encode MyUnion with kind %i",                                 \
              static_cast<KIND_UNDERLYING_TYPE>(result.kind)                           \
          );                                                                           \
          return false;                                                                \
        }                                                                              \
          BIND_MAP_2(                                                                  \
              H_INDIRECT_WRITE_UNION_DECODE, CAT(UNION_TYPE, Kind), EMPTY, __VA_ARGS__ \
          )                                                                            \
      }                                                                                \
                                                                                       \
      return true;                                                                     \
    }
#endif  //! defined(H_DEF_YAML_CONVERSION_OF_UNION)

#endif  // __hemlock_io_yaml_converters_union_hpp

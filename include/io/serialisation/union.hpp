#ifndef __hemlock_io_serialisation_union_hpp
#define __hemlock_io_serialisation_union_hpp

#include "io/serialisation/enum.hpp"
#include "io/yaml.hpp"

/****************************************\
 * Serialisable union decl entry kinds. *
 *   Note that only POD structs and     *
 *   singular fields of POD/non-POD     *
 *   type are allowed.                  *
\****************************************/

#if !defined(H_POD_STRUCT)
#  define H_POD_STRUCT 1
#endif  // !defined(H_POD_STRUCT)

#if !defined(H_NON_POD_TYPE)
#  define H_NON_POD_TYPE 0
#endif  // !defined(H_NON_POD_TYPE)

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

#if !defined(H_DECL_UNION_WITH_SERIALISATION)

/************************************\
 * Serialisable union decl helpers. *
\************************************/

#  if !defined(H_UNION_ENTRY_NAME)
#    define H_UNION_ENTRY_NAME(IS_POD_STRUCT, NAME, ...) NAME
#  endif  //! defined(H_UNION_ENTRY_NAME)

#  if !defined(H_INDIRECT_UNION_ENTRY_NAME)
#    define H_INDIRECT_UNION_ENTRY_NAME(ARGS) H_UNION_ENTRY_NAME ARGS
#  endif  //! defined(H_INDIRECT_UNION_ENTRY_NAME)

#  define H_UNION_ENTRY_TO_FIELD(FIELD_DATA)                                           \
    H_FIELD_TYPE FIELD_DATA H_FIELD_NAME FIELD_DATA
#  define H_WRITE_UNION_ENTRY_KIND(ENTRY) H_UNION_ENTRY_NAME ENTRY,

#  if !defined(H_WRITE_UNION_POD_STRUCT_ENTRY)
#    define H_WRITE_UNION_POD_STRUCT_ENTRY(...)                                        \
      struct {                                                                         \
        MAP(H_UNION_ENTRY_TO_FIELD, SEMICOLON, __VA_ARGS__);                           \
      };
#  endif  // !defined(H_WRITE_UNION_POD_STRUCT_ENTRY)

#  if !defined(H_WRITE_UNION_NON_POD_TYPE_ENTRY)
#    define H_WRITE_UNION_NON_POD_TYPE_ENTRY(FIELD_DATA)                               \
      H_UNION_ENTRY_TO_FIELD(FIELD_DATA);
#  endif  // !defined(H_WRITE_UNION_NON_POD_TYPE_ENTRY)

#  if !defined(H_WRITE_UNION_ENTRY)
#    define H_WRITE_UNION_ENTRY(IS_POD_STRUCT, NAME, ...)                              \
      IF_ELSE(BOOL(IS_POD_STRUCT))                                                     \
      (H_WRITE_UNION_POD_STRUCT_ENTRY(__VA_ARGS__),                                    \
       H_WRITE_UNION_NON_POD_TYPE_ENTRY(__VA_ARGS__))
#  endif  // !defined(H_WRITE_UNION_ENTRY)

#  if !defined(H_INDIRECT_WRITE_UNION_ENTRY)
#    define H_INDIRECT_WRITE_UNION_ENTRY(ARGS) H_WRITE_UNION_ENTRY ARGS
#  endif  // !defined(H_INDIRECT_WRITE_UNION_ENTRY)

/****************************************\
 * Serialisable union decl entry macro. *
\****************************************/

#  define H_DECL_UNION_WITH_SERIALISATION(NAMESPACE, NAME, KIND_TYPE, ...)             \
    namespace NAMESPACE {                                                              \
      enum class NAME##Kind : KIND_TYPE{                                               \
          MAP(H_WRITE_UNION_ENTRY_KIND, EMPTY, __VA_ARGS__) SENTINEL                   \
      };                                                                               \
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
    H_DECL_YAML_CONVERSION_OF_UNION(NAMESPACE::NAME)
#endif  // !defined(H_DECL_UNION_WITH_SERIALISATION)

#if !defined(H_DEF_UNION_WITH_SERIALISATION)
#  define H_DEF_UNION_WITH_SERIALISATION(NAMESPACE, NAME, KIND_TYPE, ...)              \
    H_DEF_SERIALISATION_OF_ENUM(NAMESPACE, NAME##Kind)                                 \
    H_DEF_YAML_CONVERSION_OF_UNION(NAMESPACE::NAME, KIND_TYPE, __VA_ARGS__)
#endif  // !defined(H_DEF_UNION_WITH_SERIALISATION)

#endif  // __hemlock_io_serialisation_union_hpp

#ifndef __hemlock_io_serialisation_struct_hpp
#define __hemlock_io_serialisation_struct_hpp

#include "io/yaml/converters/struct.hpp"

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

#if !defined(H_DECL_STRUCT_WITH_SERIALISATION)

/*************************************\
 * Serialisable struct decl helpers. *
\*************************************/

#  if !defined(H_WRITE_STRUCT_ENTRY)
#    define H_WRITE_STRUCT_ENTRY(ENTRY) H_FIELD_TYPE ENTRY H_FIELD_NAME ENTRY
#  endif  //! defined(H_WRITE_STRUCT_ENTRY)

/****************************************\
 * Serialisable struct decl entry macro. *
\****************************************/

#  define H_DECL_STRUCT_WITH_SERIALISATION(NAMESPACE, NAME, ...)                       \
    namespace NAMESPACE {                                                              \
      struct NAME {                                                                    \
        MAP(H_WRITE_STRUCT_ENTRY, SEMICOLON, __VA_ARGS__);                             \
      };                                                                               \
    }                                                                                  \
    H_DECL_YAML_CONVERSION_OF_STRUCT(NAMESPACE, NAME)
#endif  // !defined(H_DECL_STRUCT_WITH_SERIALISATION)

#if !defined(H_DEF_STRUCT_WITH_SERIALISATION)
#  define H_DEF_STRUCT_WITH_SERIALISATION(NAMESPACE, NAME, ...)                        \
    H_DEF_YAML_CONVERSION_OF_STRUCT(NAMESPACE, NAME, __VA_ARGS__)
#endif  // !defined(H_DEF_STRUCT_WITH_SERIALISATION)

#endif  // __hemlock_io_serialisation_struct_hpp

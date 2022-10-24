#ifndef __hemlock_io_serialisation_struct_hpp
#define __hemlock_io_serialisation_struct_hpp

#include "io/yaml/converters/struct.hpp"

#if !defined(H_DECL_STRUCT_WITH_SERIALISATION)
#  define H_DECL_STRUCT_WITH_SERIALISATION(NAMESPACE, NAME)
#endif // !defined(H_DECL_STRUCT_WITH_SERIALISATION)

#if !defined(H_DEF_STRUCT_WITH_SERIALISATION)
#  define H_DEF_STRUCT_WITH_SERIALISATION(NAMESPACE, NAME, ...) \
    H_DEF_STRUCT_WITH_YAML_CONVERSION(NAMESPACE, NAME, __VA_ARGS__)
#endif // !defined(H_DEF_STRUCT_WITH_SERIALISATION)

#endif // __hemlock_io_serialisation_struct_hpp

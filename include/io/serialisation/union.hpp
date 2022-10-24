#ifndef __hemlock_io_serialisation_union_hpp
#define __hemlock_io_serialisation_union_hpp

#include "io/yaml/converters/union.hpp"

#if !defined(H_DECL_UNION_WITH_SERIALISATION)
#  define H_DECL_UNION_WITH_SERIALISATION(NAMESPACE, NAME)
#endif // !defined(H_DECL_UNION_WITH_SERIALISATION)

#if !defined(H_DEF_UNION_WITH_SERIALISATION)
#  define H_DEF_UNION_WITH_SERIALISATION(NAMESPACE, NAME, KIND_TYPE, ...) \
    H_DEF_UNION_WITH_YAML_CONVERSION(NAMESPACE, NAME, KIND_TYPE, __VA_ARGS__)
#endif // !defined(H_DEF_UNION_WITH_SERIALISATION)

#endif // __hemlock_io_serialisation_union_hpp

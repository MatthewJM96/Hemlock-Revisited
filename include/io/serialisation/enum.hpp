#ifndef __hemlock_io_serialisation_enum_hpp
#define __hemlock_io_serialisation_enum_hpp

namespace hemlock {
    namespace io {
        template <typename Type>
        constexpr const char* serialisable_enum_name(Type val);

        template <typename Type>
        constexpr Type serialisable_enum_val(const std::string& name);
    }
}
namespace hio = hemlock::io;

#if !defined(H_DECL_SERIALISABLE_ENUM)
#  define H_DECL_SERIALISABLE_ENUM(NAME) enum class NAME;
#endif //!defined(H_DECL_SERIALISABLE_ENUM)

#if !defined(H_DEF_SERIALISABLE_ENUM)
#  if !defined(H_SERIALISABLE_ENUM_ENTRY)
#    define H_SERIALISABLE_ENUM_ENTRY(ENTRY_NAME, ID)               \
ENTRY_NAME = ID_TO_INT(#ID)
#  endif //!defined(H_SERIALISABLE_ENUM_ENTRY)

#  define H_DEF_SERIALISABLE_ENUM(NAME, ...)                        \
enum class NAME {                                                   \
    MAP_WITH_ID(H_SERIALISABLE_ENUM_ENTRY, COMMA, __VA_ARGS__),     \
    SENTINEL                                                        \
};
#endif //!defined(H_DEF_SERIALISABLE_ENUM)

#if !defined(H_DECL_SERIALISATION_OF_ENUM)
#  if !defined(H_ENUM_NAME)
#    define H_ENUM_NAME(ENTRY) #ENTRY
#  endif //!defined(H_ENUM_NAME)
#  if !defined(H_ENUM_VAL)
#    define H_ENUM_VAL(NAMESPACE, ENTRY) { #ENTRY, NAMESPACE :: ENTRY }
#  endif //!defined(H_ENUM_VAL)

#  define H_DECL_SERIALISATION_OF_ENUM(NAMESPACE, NAME, ...)                                    \
namespace hemlock {                                                                             \
    namespace io {                                                                              \
        template <>                                                                             \
        const char* serialisable_enum_name<NAMESPACE :: NAME>(NAMESPACE :: NAME val);           \
                                                                                                \
        template <>                                                                             \
        NAMESPACE :: NAME serialisable_enum_val<NAMESPACE :: NAME>(const std::string& name);    \
    }                                                                                           \
}
#endif // !defined(H_DECL_SERIALISATION_OF_ENUM)

#if !defined(H_DEF_SERIALISATION_OF_ENUM)
#  if !defined(H_ENUM_NAME)
#    define H_ENUM_NAME(ENTRY) #ENTRY
#  endif //!defined(H_ENUM_NAME)
#  if !defined(H_ENUM_VAL)
#    define H_ENUM_VAL(NAMESPACE, ENTRY) { #ENTRY, NAMESPACE :: ENTRY }
#  endif //!defined(H_ENUM_VAL)

#  define H_DEF_SERIALISATION_OF_ENUM(NAMESPACE, NAME, ...)                                     \
namespace hemlock {                                                                             \
    namespace io {                                                                              \
        const char* NAME##_Names[] = {                                                          \
            MAP(H_ENUM_NAME, COMMA, __VA_ARGS__)                                                \
        };                                                                                      \
                                                                                                \
        const std::unordered_map<std::string, NAMESPACE :: NAME> NAME##_Values = {              \
            BIND_MAP(H_ENUM_VAL, NAMESPACE :: NAME, COMMA, __VA_ARGS__)                         \
        };                                                                                      \
                                                                                                \
        template <>                                                                             \
        const char* serialisable_enum_name<NAMESPACE :: NAME>(NAMESPACE :: NAME val) {          \
            return NAME##_Names[                                                                \
                static_cast<size_t>(val)                                                        \
            ];                                                                                  \
        }                                                                                       \
                                                                                                \
        template <>                                                                             \
        NAMESPACE :: NAME serialisable_enum_val<NAMESPACE :: NAME>(const std::string& name) {   \
            auto it = NAME##_Values.find(name);                                                 \
            if (it != NAME##_Values.end()) {                                                    \
                return it->second;                                                              \
            }                                                                                   \
            return NAMESPACE :: NAME::SENTINEL;                                                 \
        }                                                                                       \
    }                                                                                           \
}
#endif // !defined(H_DEF_SERIALISATION_OF_ENUM)


#if !defined(H_DECL_ENUM_WITH_SERIALISATION)
#  define H_DECL_ENUM_WITH_SERIALISATION(NAMESPACE, NAME, ...)  \
namespace NAMESPACE {                                           \
    H_DEF_SERIALISABLE_ENUM(NAME, __VA_ARGS__)                  \
}                                                               \
H_DECL_SERIALISATION_OF_ENUM(NAMESPACE, NAME, __VA_ARGS__)
#endif //!defined(H_DECL_ENUM_WITH_SERIALISATION)

#if !defined(H_DEF_ENUM_WITH_SERIALISATION)
#  define H_DEF_ENUM_WITH_SERIALISATION(NAMESPACE, NAME, ...)   \
H_DEF_SERIALISATION_OF_ENUM(NAMESPACE, NAME, __VA_ARGS__)
#endif //!defined(H_DEF_ENUM_WITH_SERIALISATION)

#endif // __hemlock_io_serialisation_enum_hpp

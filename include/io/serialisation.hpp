#ifndef __hemlock_io_serialisation_hpp
#define __hemlock_io_serialisation_hpp

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
#define H_DECL_SERIALISABLE_ENUM(NAME) enum class NAME;
#endif //!defined(H_DECL_SERIALISABLE_ENUM)

#if !defined(H_DEF_SERIALISABLE_ENUM)
#define H_SERIALISABLE_ENUM_ENTRY(ENTRY_NAME, ID)                   \
ENTRY_NAME = ID_TO_INT(#ID)

#define H_DEF_SERIALISABLE_ENUM(NAME, ...)                          \
enum class NAME {                                                   \
    MAP_WITH_ID(H_SERIALISABLE_ENUM_ENTRY, COMMA, __VA_ARGS__),     \
    SENTINEL                                                        \
};
#endif //!defined(H_DEF_SERIALISABLE_ENUM)

#if !defined(H_DEF_ENUM_SERIALISATION)
#define H_SERIALISABLE_ENUM_NAME(NAME, VAL) NAME
#define H_SERIALISABLE_ENUM_VAL(NAME, VAL) { NAME, VAL }

#define H_DEF_ENUM_SERIALISATION(NAME, ...)                                     \
namespace hemlock {                                                             \
    namespace io {                                                              \
        const char* NAME_Names[] = {                                            \
            MAP(H_SERIALISABLE_ENUM_NAME, COMMA, __VA_ARGS__)                   \
        };                                                                      \
                                                                                \
        const std::unordered_map<std::string, NAME> NAME_Values = {             \
            MAP(H_SERIALISABLE_ENUM_VAL, COMMA, __VA_ARGS__)                    \
        };                                                                      \
                                                                                \
        template <>                                                             \
        constexpr const char* serialisable_enum_name<NAME>(NAME val) {          \
            return NAME_Names[                                                  \
                static_cast<size_t>(val)                                        \
            ];                                                                  \
        }                                                                       \
                                                                                \
        template <>                                                             \
        NAME serialisable_enum_val<NAME>(const std::string& name) {             \
            auto it = NAME_Vals.find(name);                                     \
            if (it != NAME_Vals.end()) {                                        \
                return it->second;                                              \
            }                                                                   \
            return NAME::SENTINEL;                                              \
        }                                                                       \
    }                                                                           \
}
#endif // !defined(H_DEF_ENUM_SERIALISATION)

#endif // __hemlock_io_serialisation_hpp

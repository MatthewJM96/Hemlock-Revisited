#ifndef __hemlock_io_serialisation_enum_hpp
#define __hemlock_io_serialisation_enum_hpp

#if !defined(H_DECL_SERIALISABLE_ENUM)
#  define H_DECL_SERIALISABLE_ENUM(NAME) enum class NAME;
#endif  //! defined(H_DECL_SERIALISABLE_ENUM)

#if !defined(H_DEF_SERIALISABLE_ENUM)
#  if !defined(H_SERIALISABLE_ENUM_ENTRY)
#    define H_SERIALISABLE_ENUM_ENTRY(ENTRY_NAME) ENTRY_NAME
#  endif  //! defined(H_SERIALISABLE_ENUM_ENTRY)

#  define H_DEF_SERIALISABLE_ENUM(NAME, TYPE, ...)                                     \
    enum class NAME : TYPE {                                                           \
      MAP(H_SERIALISABLE_ENUM_ENTRY, COMMA, __VA_ARGS__),                              \
      SENTINEL                                                                         \
    };
#endif  //! defined(H_DEF_SERIALISABLE_ENUM)

#if !defined(H_DEF_SERIALISABLE_VENUM)
#  if !defined(H_VENUM_NAME)
#    define H_VENUM_NAME(NAME, VALUE) NAME
#  endif  //! defined(H_VENUM_NAME)

#  if !defined(H_SERIALISABLE_VENUM_ENTRY)
#    define H_SERIALISABLE_VENUM_ENTRY(ENTRY) H_VENUM_NAME ENTRY
#  endif  //! defined(H_SERIALISABLE_ENUM_ENTRY)

#  define H_DEF_SERIALISABLE_VENUM(NAME, TYPE, ...)                                    \
    enum class NAME : TYPE {                                                           \
      MAP(H_SERIALISABLE_VENUM_ENTRY, COMMA, __VA_ARGS__),                             \
      SENTINEL                                                                         \
    };
#endif  //! defined(H_DEF_SERIALISABLE_VENUM)

#if !defined(H_DECL_SERIALISATION_OF_ENUM)
#  if !defined(H_ENUM_NAME)
#    define H_ENUM_NAME(ENTRY) #ENTRY
#  endif  //! defined(H_ENUM_NAME)
#  if !defined(H_ENUM_VAL)
#    define H_ENUM_VAL(NAMESPACE, ENTRY)                                               \
      {                                                                                \
#        ENTRY, NAMESPACE ::ENTRY                                                      \
      }
#  endif  //! defined(H_ENUM_VAL)

#  define H_DECL_SERIALISATION_OF_ENUM(NAMESPACE, NAME, ...)                           \
    namespace hemlock {                                                                \
      namespace io {                                                                   \
        [[maybe_unused]] static const std::                                            \
            unordered_map<std::string, NAMESPACE ::NAME>                               \
                NAME##_Values                                                          \
            = { BIND_MAP(H_ENUM_VAL, NAMESPACE ::NAME, COMMA, __VA_ARGS__) };          \
                                                                                       \
        [[maybe_unused]] static const char* NAME##_Names[]                             \
            = { MAP(H_ENUM_NAME, COMMA, __VA_ARGS__) };                                \
                                                                                       \
        template <>                                                                    \
        const char* serialisable_enum_name<NAMESPACE ::NAME>(NAMESPACE ::NAME val);    \
                                                                                       \
        template <>                                                                    \
        NAMESPACE ::NAME                                                               \
        serialisable_enum_val<NAMESPACE ::NAME>(const std::string& name);              \
      }                                                                                \
    }
#endif  // !defined(H_DECL_SERIALISATION_OF_ENUM)

#if !defined(H_DECL_SERIALISATION_OF_VENUM)
#  if !defined(H_VENUM_NAME)
#    define H_VENUM_NAME(NAME, VALUE) NAME
#  endif  //! defined(H_VENUM_NAME)
#  if !defined(H_VENUM_NAME_STR)
#    define H_VENUM_NAME_STR(NAME, VALUE) #NAME
#  endif  //! defined(H_VENUM_NAME_STR)
#  if !defined(H_VENUM_MAP_ENTRY)
#    define H_VENUM_MAP_ENTRY(NAMESPACE, ENTRY)                                        \
      {                                                                                \
        H_VENUM_NAME_STR ENTRY, NAMESPACE ::H_VENUM_NAME ENTRY                         \
      }
#  endif  //! defined(H_VENUM_MAP_ENTRY)
#  if !defined(H_VENUM_ARR_ENTRY)
#    define H_VENUM_ARR_ENTRY(ENTRY) H_VENUM_NAME_STR ENTRY
#  endif  //! defined(H_VENUM_ARR_ENTRY)

#  define H_DECL_SERIALISATION_OF_VENUM(NAMESPACE, NAME, ...)                          \
    namespace hemlock {                                                                \
      namespace io {                                                                   \
        [[maybe_unused]] static const std::                                            \
            unordered_map<std::string, NAMESPACE ::NAME>                               \
                NAME##_Values                                                          \
            = { BIND_MAP(H_VENUM_MAP_ENTRY, NAMESPACE ::NAME, COMMA, __VA_ARGS__) };   \
                                                                                       \
        [[maybe_unused]] static const char* NAME##_Names[]                             \
            = { MAP(H_VENUM_ARR_ENTRY, COMMA, __VA_ARGS__) };                          \
                                                                                       \
        template <>                                                                    \
        const char* serialisable_enum_name<NAMESPACE ::NAME>(NAMESPACE ::NAME val);    \
                                                                                       \
        template <>                                                                    \
        NAMESPACE ::NAME                                                               \
        serialisable_enum_val<NAMESPACE ::NAME>(const std::string& name);              \
      }                                                                                \
    }
#endif  // !defined(H_DECL_SERIALISATION_OF_VENUM)

#if !defined(H_DEF_SERIALISATION_OF_ENUM)
#  if !defined(H_ENUM_NAME)
#    define H_ENUM_NAME(ENTRY) #ENTRY
#  endif  //! defined(H_ENUM_NAME)
#  if !defined(H_ENUM_VAL)
#    define H_ENUM_VAL(NAMESPACE, ENTRY)                                               \
      {                                                                                \
#        ENTRY, NAMESPACE ::ENTRY                                                      \
      }
#  endif  //! defined(H_ENUM_VAL)

#  define H_DEF_SERIALISATION_OF_ENUM(NAMESPACE, NAME)                                 \
    namespace hemlock {                                                                \
      namespace io {                                                                   \
        template <>                                                                    \
        [[maybe_unused]] const char*                                                   \
        serialisable_enum_name<NAMESPACE ::NAME>(NAMESPACE ::NAME val) {               \
          return NAME##_Names[static_cast<size_t>(val)];                               \
        }                                                                              \
                                                                                       \
        template <>                                                                    \
        NAMESPACE ::NAME [[maybe_unused]] serialisable_enum_val<NAMESPACE ::NAME>(     \
            const std::string& name                                                    \
        ) {                                                                            \
          auto it = NAME##_Values.find(name);                                          \
          if (it != NAME##_Values.end()) {                                             \
            return it->second;                                                         \
          }                                                                            \
          return NAMESPACE ::NAME::SENTINEL;                                           \
        }                                                                              \
      }                                                                                \
    }
#endif  // !defined(H_DEF_SERIALISATION_OF_ENUM)

#if !defined(H_DECL_ENUM_WITH_SERIALISATION)
#  define H_DECL_ENUM_WITH_SERIALISATION(NAMESPACE, NAME, TYPE, ...)                   \
    namespace NAMESPACE {                                                              \
      H_DEF_SERIALISABLE_ENUM(NAME, TYPE, __VA_ARGS__)                                 \
    }                                                                                  \
    H_DECL_SERIALISATION_OF_ENUM(NAMESPACE, NAME, __VA_ARGS__)
#endif  //! defined(H_DECL_ENUM_WITH_SERIALISATION)

#if !defined(H_DECL_VENUM_WITH_SERIALISATION)
#  define H_DECL_VENUM_WITH_SERIALISATION(NAMESPACE, NAME, TYPE, ...)                  \
    namespace NAMESPACE {                                                              \
      H_DEF_SERIALISABLE_VENUM(NAME, TYPE, __VA_ARGS__)                                \
    }                                                                                  \
    H_DECL_SERIALISATION_OF_VENUM(NAMESPACE, NAME, __VA_ARGS__)
#endif  //! defined(H_DECL_VENUM_WITH_SERIALISATION)

#if !defined(H_DEF_ENUM_WITH_SERIALISATION)
#  define H_DEF_ENUM_WITH_SERIALISATION(NAMESPACE, NAME)                               \
    H_DEF_SERIALISATION_OF_ENUM(NAMESPACE, NAME)
#endif  //! defined(H_DEF_ENUM_WITH_SERIALISATION)

#endif  // __hemlock_io_serialisation_enum_hpp

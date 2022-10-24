#ifndef __hemlock_io_yaml_converters_union_hpp
#define __hemlock_io_yaml_converters_union_hpp

// TODO(Matthew): Fix recursion of MAP not working. Note that
//                MAP - MAP results in interior MAP not being
//                evaluated, while BIND_MAP - MAP results in
//                the op of the interior MAP not being
//                evaluated.

#if !defined(H_DEF_YAML_CONVERSION_OF_UNION)
#  define H_UNION_SUBENTRY_NAME(NAME, TYPE) #NAME
#  define H_UNION_SUBENTRY_FIELD(NAME, TYPE) NAME
#  define H_UNION_SUBENTRY_TYPE(NAME, TYPE) TYPE

#  if !defined(H_UNION_SUBENTRY_ENCODE)
#    define H_UNION_SUBENTRY_ENCODE(SUBENTRY_INFO)          \
node[H_UNION_SUBENTRY_NAME SUBENTRY_INFO] = YAML::Node(     \
    val.H_UNION_SUBENTRY_FIELD SUBENTRY_INFO                \
)
#  endif //!defined(H_UNION_SUBENTRY_ENCODE)

#  if !defined(H_UNION_SUBENTRY_DECODE)
#    define H_UNION_SUBENTRY_DECODE(SUBENTRY_INFO)          \
if (                                                        \
    !convert<H_UNION_SUBENTRY_TYPE SUBENTRY_INFO>::decode(  \
        node[H_UNION_SUBENTRY_NAME SUBENTRY_INFO],          \
        result.H_UNION_SUBENTRY_FIELD SUBENTRY_INFO         \
    )                                                       \
) {                                                         \
    return false;                                           \
}
#  endif //!defined(H_UNION_SUBENTRY_DECODE)

#  define H_UNION_ENTRY_FIELD(NAME, ...) NAME
#  define H_UNION_ENTRY_SUBENTRIES(NAME, ...) __VA_ARGS__

#  if !defined(H_UNION_ENTRY_ENCODE)
#    define H_UNION_ENTRY_ENCODE(UNION_TYPE, ENTRY_INFO)    \
case UNION_TYPE##Kind :: H_UNION_ENTRY_FIELD ENTRY_INFO :   \
{                                                           \
    MAP(                                                    \
        H_UNION_SUBENTRY_ENCODE,                            \
        SEMICOLON,                                          \
        H_UNION_ENTRY_SUBENTRIES ENTRY_INFO                 \
    );                                                      \
}
#  endif //!defined(H_UNION_ENTRY_ENCODE)

#  if !defined(H_UNION_ENTRY_DECODE)
#    define H_UNION_ENTRY_DECODE(UNION_TYPE, ENTRY_INFO)    \
case UNION_TYPE##Kind :: H_UNION_ENTRY_FIELD ENTRY_INFO :   \
{                                                           \
    MAP(                                                    \
        H_UNION_SUBENTRY_DECODE,                            \
        EMPTY,                                              \
        H_UNION_ENTRY_SUBENTRIES ENTRY_INFO                 \
    )                                                       \
}
#  endif //!defined(H_UNION_ENTRY_DECODE)


#  define H_DEF_YAML_CONVERSION_OF_UNION(UNION_TYPE, KIND_TYPE, ...)    \
namespace YAML {                                                        \
    template <>                                                         \
    struct convert<UNION_TYPE> {                                        \
        static Node encode(const TYPE& val) {                           \
            YAML::Node node(YAML::NodeType::Map);                       \
                                                                        \
            node["kind"] = YAML::Node(val.kind);                        \
                                                                        \
            switch (val.kind) {                                         \
                BIND_MAP(                                               \
                    H_UNION_ENTRY_ENCODE,                               \
                    UNION_TYPE,                                         \
                    EMPTY,                                              \
                    __VA_ARGS__                                         \
                )                                                       \
                default:                                                \
                    debug_printf(                                       \
                        "Could not encode MyUnion with kind %i",        \
                        static_cast<KIND_TYPE>(val.kind)                \
                    );                                                  \
                    return YAML::Node();                                \
            }                                                           \
                                                                        \
            return node;                                                \
        }                                                               \
                                                                        \
        static bool decode(const Node& node, UNION_TYPE& result) {      \
            if (                                                        \
                !convert<UNION_TYPE##Kind>::decode(                     \
                    node["kind"],                                       \
                    result.kind                                         \
                )                                                       \
            ) {                                                         \
                return false;                                           \
            }                                                           \
                                                                        \
            switch (result.kind) {                                      \
                BIND_MAP(                                               \
                    H_UNION_ENTRY_DECODE,                               \
                    UNION_TYPE,                                         \
                    EMPTY,                                              \
                    __VA_ARGS__                                         \
                )                                                       \
                default:                                                \
                    debug_printf(                                       \
                        "Could not encode MyUnion with kind %i",        \
                        static_cast<KIND_TYPE>(val.kind)                \
                    );                                                  \
                    return false;                                       \
            }                                                           \
                                                                        \
            return true;                                                \
        }                                                               \
    };                                                                  \
}
#  endif //!defined(H_DEF_YAML_CONVERSION_OF_UNION)

#if !defined(H_DEF_UNION_WITH_YAML_CONVERSION)
#  if !defined(H_UNION_SUBENTRY_FIELD)
#    define H_UNION_SUBENTRY_FIELD(NAME, TYPE) NAME
#  endif //!defined(H_UNION_SUBENTRY_FIELD)
#  if !defined(H_UNION_SUBENTRY_TYPE)
#    define H_UNION_SUBENTRY_TYPE(NAME, TYPE) TYPE
#  endif //!defined(H_UNION_SUBENTRY_TYPE)
#  if !defined(H_UNION_ENTRY_FIELD)
#    define H_UNION_ENTRY_FIELD(NAME, ...) NAME
#  endif //!defined(H_UNION_ENTRY_FIELD)
#  if !defined(H_UNION_ENTRY_SUBENTRIES)
#    define H_UNION_ENTRY_SUBENTRIES(NAME, ...) __VA_ARGS__
#  endif //!defined(H_UNION_ENTRY_SUBENTRIES)

#  define H_UNION_SUBENTRY_TO_FIELDS(SUBENTRY) \
H_UNION_SUBENTRY_TYPE SUBENTRY H_UNION_SUBENTRY_FIELD SUBENTRY

#  define H_UNION_ENTRY_TO_SUBENTRIES(ENTRY)    \
struct {                                        \
    MAP(                                        \
        H_UNION_SUBENTRY_TO_FIELDS,             \
        SEMICOLON,                              \
        H_UNION_ENTRY_SUBENTRIES ENTRY          \
    )                                           \
}

#  define H_UNION_ENTRY_TO_KIND(ENTRY) H_UNION_ENTRY_FIELD ENTRY

#  define H_DEF_UNION_WITH_YAML_CONVERSION(NAMESPACE, NAME, KIND_TYPE, ...) \
namespace NAMESPACE {                                                       \
    enum class NAME##Kind : KIND_TYPE {                                     \
        MAP(H_UNION_ENTRY_TO_KIND, COMMA, __VA_ARGS__),                     \
        SENTINEL                                                            \
    };                                                                      \
    struct NAME {                                                           \
        NAME##Kind kind;                                                    \
        union {                                                             \
            MAP(H_UNION_ENTRY_TO_SUBENTRIES, SEMICOLON, __VA_ARGS__);       \
        };                                                                  \
    };                                                                      \
}                                                                           \
H_DEF_YAML_CONVERSION_OF_UNION(NAMESPACE :: NAME, KIND_TYPE, __VA_ARGS__)
#endif //!defined(H_DEF_UNION_WITH_YAML_CONVERSION)

#endif // __hemlock_io_yaml_converters_union_hpp

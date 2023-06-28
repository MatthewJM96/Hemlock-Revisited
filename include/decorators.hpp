// Memory Management
#define CALLER_DELETE
#define CALLEE_DELETE

// Data Direction
#define OUT
#define IN

#if defined(DEBUG)
#  define debug_printf(...) printf(__VA_ARGS__)
#else
#  define debug_printf(...)
#  if !defined(NDEBUG)
#    define NDEBUG
#  endif  // !defined(NDEBUG)
#endif

#define HEMLOCK_NON_COPYABLE(TYPE)                                                     \
  TYPE(const TYPE& rhs)            = delete;                                           \
  TYPE& operator=(const TYPE& rhs) = delete

#define HEMLOCK_COPYABLE(TYPE)                                                         \
  TYPE(const TYPE& rhs) { *this = rhs; }                                               \
  TYPE& operator=(const TYPE& rhs)

#define HEMLOCK_NON_MOVABLE(TYPE)                                                      \
  TYPE(TYPE&& rhs)            = delete;                                                \
  TYPE& operator=(TYPE&& rhs) = delete

#define HEMLOCK_MOVABLE(TYPE)                                                          \
  TYPE(TYPE&& rhs) { *this = std::forward<TYPE>(rhs); }                                \
  TYPE& operator=(TYPE&& rhs)

#if defined(HEMLOCK_COMPILER_GCC) || defined(HEMLOCK_COMPILER_CLANG)
#  define HEMLOCK_PACKED_STRUCT(DECL) DECL __attribute__((packed))
#  define HEMLOCK_NOINLINE            __attribute__((noinline))
#else  // defined(HEMLOCK_COMPILER_GCC) || defined(HEMLOCK_COMPILER_CLANG)
#  define HEMLOCK_PACKED_STRUCT(DECL) __pragma(pack(push, 1)) DECL __pragma(pack(pop))
#  define HEMLOCK_NOINLINE            __declspec(noinline)
#endif

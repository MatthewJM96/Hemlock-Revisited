// Memory Management
#define CALLER_DELETE
#define CALLEE_DELETE

// Data Direction
#define OUT
#define IN

#if defined(DEBUG)
    #define debug_printf(...) printf(__VA_ARGS__)
#else
    #define debug_printf(...)
    #if !defined(NDEBUG)
        #define NDEBUG
    #endif // !defined(NDEBUG)
#endif

#define H_NON_COPYABLE(TYPE)        \
    TYPE(const TYPE& rhs) = delete; \
    TYPE& operator=(const TYPE& rhs) = delete

#define H_COPYABLE(TYPE)        \
    TYPE(const TYPE& rhs) { *this = rhs; } \
    TYPE& operator=(const TYPE& rhs)

#define H_NON_MOVABLE(TYPE)        \
    TYPE(TYPE&& rhs) = delete; \
    TYPE& operator=(TYPE&& rhs) = delete

#define H_MOVABLE(TYPE)        \
    TYPE(TYPE&& rhs) { *this = std::forward<TYPE>(rhs); } \
    TYPE& operator=(TYPE&& rhs)

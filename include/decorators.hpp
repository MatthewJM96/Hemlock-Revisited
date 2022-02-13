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

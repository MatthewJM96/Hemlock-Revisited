#if defined(HEMLOCK_USING_VULKAN)
    #if defined(DEBUG)
        #if !defined(CREATE_VK_VALIDATION_LAYERS)
            #define CREATE_VK_VALIDATION_LAYERS const std::vector<const char*> __VK_VALIDATION_LAYERS = { \
                                                    "VK_LAYER_KHRONOS_validation"                         \
                                                }
        #endif // !defined(CREATE_VK_VALIDATION_LAYERS)
        #define VK_VALIDATION_LAYERS __VK_VALIDATION_LAYERS.data()
        #if !defined(ENABLE_VK_VALIDATION_LAYERS)
            #define ENABLE_VK_VALIDATION_LAYERS true
        #endif
    #else // defined(DEBUG)
        #if !defined(CREATE_VK_VALIDATION_LAYERS)
            #define CREATE_VK_VALIDATION_LAYERS
        #endif // !defined(CREATE_VK_VALIDATION_LAYERS)
        #define VK_VALIDATION_LAYERS nullptr
        #if !defined(ENABLE_VK_VALIDATION_LAYERS)
            #define ENABLE_VK_VALIDATION_LAYERS false
        #endif
    #endif
#endif // defined(HEMLOCK_USING_VULKAN)

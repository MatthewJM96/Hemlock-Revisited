#ifndef __hemlock_compat_hpp
#define __hemlock_compat_hpp

/**************\
 * OS Defines *
\**************/
 
#if defined(_WIN32)
#   define HEMLOCK_OS_WINDOWS
#endif // defined(_WIN32)

#if defined(__linux__)
#    define HEMLOCK_OS_LINUX
#endif

#if defined(__APPLE__)
#    define HEMLOCK_OS_MAC
#endif

/***************************\
 * Compiler & Arch Defines *
\***************************/

#if defined(_WIN64)
#   define HEMLOCK_ARCH_64
#elif defined(_WIN32)
#   define HEMLOCK_ARCH_32
#endif // defined(_WIN32)

#if defined(_MSC_VER)
#   define HEMLOCK_COMPILER_MSVC
#elif defined(__clang__) // defined(_MSC_VER)
#   define HEMLOCK_COMPILER_CLANG
#   if defined(__i386__)
#       if defined(__x86_64__)
#           define HEMLOCK_ARCH_64
#       else // defined(__x86_64__)
#           define HEMLOCK_ARCH_32
#       endif // defined(__x86_64__)
#   endif // defined(__i386__)
#elif defined(__GNUC__) || defined(__GNUG__) // defined(__clang__)
#   define HEMLOCK_COMPILER_GCC
#   if defined(__i386__)
#       if defined(__x86_64__)
#           define HEMLOCK_ARCH_64
#       else
#           define HEMLOCK_ARCH_32
#       endif // defined(__x86_64__)
#   endif // defined(__i386__)
#endif // defined(__GNUC__) || defined(__GNUG__)

#endif // __hemlock_compat_hpp

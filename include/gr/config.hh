#pragma once

#if defined(_MSVC_LANG)
    #define GR_CPP_VERSION _MSVC_LANG
#else
    #define GR_CPP_VERSION __cplusplus
#endif

#if GR_CPP_VERSION >= 202302L
    #define GR_HAS_CPP23 1
    #define GR_HAS_STD_FORMATTER 1
#else
    #define GR_HAS_CPP23 0
    #define GR_HAS_STD_FORMATTER 0
#endif

// C++17 检测
#if GR_CPP_VERSION >= 201703L
    #define GR_HAS_CPP17 1
#else
    #define GR_HAS_CPP17 0
#endif

// C++20 检测修正
#if GR_CPP_VERSION >= 202002L
    #define GR_HAS_CPP20 1
    #define GR_HAS_CONCEPTS 1
    #define GR_HAS_CHAR8_T 1
    #define GR_HAS_STD_FORMAT 1
#else
    #define GR_HAS_CPP20 0
    #define GR_HAS_CONCEPTS 0
    #define GR_HAS_CHAR8_T 0
    #define GR_HAS_STD_FORMAT 0
#endif

#if GR_CPP_VERSION >= 202302L
    #define GR_HAS_STD_FORMATTER 1
#else
    #define GR_HAS_STD_FORMATTER 0
#endif

#if GR_HAS_CPP23
    #define GR_CONSTEXPR_OR_INLINE constexpr
    #define GR_DELETE_FUNC(x) delete(x)
#else
    #define GR_CONSTEXPR_OR_INLINE inline
    #define GR_DELETE_FUNC(x) delete
#endif

#ifdef _MSC_VER
    #define GR_FORCE_INLINE __forceinline
#elif defined(__GNUC__)
    #define GR_FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define GR_FORCE_INLINE inline
#endif

#if GR_HAS_CPP20
    #define GR_CONSTEVAL consteval
#else
    #define GR_CONSTEVAL constexpr
#endif
// CONCEPT
#if GR_HAS_CONCEPTS
    #define GR_REQUIRES(...) requires __VA_ARGS__
#else
    #define GR_REQUIRES(...)
#endif

#if GR_CPP_VERSION >= 201703L
    #define GR_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
    #define GR_STATIC_ASSERT(cond, msg) static_assert(cond, #msg)
#endif

#if !GR_HAS_CHAR8_T
using char8_t = char;
#endif
#define TERMUX_CLANG_CPP 0
#define GR_HAS_RE2 1
#define GR_LOG_LEVEL 0

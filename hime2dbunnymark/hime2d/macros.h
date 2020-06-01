#pragma once

#include "platform.h"

// namespace hime2d
#define NS_HIME_BEGIN namespace hime2d {
#define NS_HIME_END }
#define USING_NS_HIME using namespace hime2d

#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define FORCE_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#define LIKELY(x) x
#define UNLIKELY(x) x
#define FORCE_INLINE __forceinline
#else
#define LIKELY(x) x
#define UNLIKELY(x) x
#define FORCE_INLINE inline
#endif

// assertion
#if HIME2D_DEBUG > 0
#include <assert.h>
#define HM_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            if (!LuaEngine::getInstance()->handleAssert(msg)) { \
                assert(cond); \
            } \  
        } \
    } while (0)
#else
#define HM_ASSERT(cond, msg)
#endif

// deletion
#define HM_SAFE_DELETE(p)           do { delete (p); (p) = nullptr; } while(0)
#define HM_SAFE_DELETE_ARRAY(p)     do { if(p) { delete[] (p); (p) = nullptr; } } while(0)
#define HM_SAFE_FREE(p)             do { if(p) { free(p); (p) = nullptr; } } while(0)


// hime2d debug macros
#include <stdio.h>
#include <string.h>

inline const char *_stripDir(const char *str) {
#if HM_TARGET_PLATFORM == HM_PLATFORM_WIN32
    const char *slash = strrchr(str, '\\');
#else
    const char *slash = strrchr(str, '/');
#endif
    return slash ? (slash + 1) : str;
}

#if HM_TARGET_PLATFORM == HM_PLATFORM_ANDROID

#include <android/log.h>
#define LOGTAG "hime2d debug info"

#if HIME2D_DEBUG > 0
#define HM_INFO(format, ...)    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "%s:%i: " format, _stripDir(__FILE__), __LINE__, __VA_ARGS__)
#else
#define HM_INFO(...)
#endif

#define HM_LOG(format, ...)     __android_log_print(ANDROID_LOG_INFO, LOGTAG, "%s:%i: " format, _stripDir(__FILE__), __LINE__, __VA_ARGS__)
#define HM_WARN(format, ...)    __android_log_print(ANDROID_LOG_WARN, LOGTAG, "%s:%i: " format, _stripDir(__FILE__), __LINE__, __VA_ARGS__)
#define HM_ERROR(format, ...)   __android_log_print(ANDROID_LOG_ERROR, LOGTAG, "%s:%i: " format, _stripDir(__FILE__), __LINE__, __VA_ARGS__)
#define HM_FATAL(format, ...)   __android_log_print(ANDROID_LOG_FATAL, LOGTAG, "%s:%i: " format, _stripDir(__FILE__), __LINE__, __VA_ARGS__)

#else

#if HIME2D_DEBUG > 0
#define HM_INFO(format, ...)    { fprintf(stdout, "I: %s:%i: " format "\n", _stripDir(__FILE__), __LINE__, ##__VA_ARGS__); fflush(stdout); }
#else
#define HM_INFO(...)
#endif

#define HM_LOG(format, ...)     { fprintf(stdout, "D: %s:%i: " format "\n", _stripDir(__FILE__), __LINE__, ##__VA_ARGS__); fflush(stdout); }
#define HM_WARN(format, ...)    { fprintf(stdout, "W: %s:%i: " format "\n", _stripDir(__FILE__), __LINE__, ##__VA_ARGS__); fflush(stdout); }
#define HM_ERROR(format, ...)   { fprintf(stdout, "E: %s:%i: " format "\n", _stripDir(__FILE__), __LINE__, ##__VA_ARGS__); fflush(stdout); }
#define HM_FATAL(format, ...)   { fprintf(stdout, "F: %s:%i: " format "\n", _stripDir(__FILE__), __LINE__, ##__VA_ARGS__); fflush(stdout); }

#endif
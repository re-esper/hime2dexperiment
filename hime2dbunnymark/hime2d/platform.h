#pragma once

// define supported target platform macro which HM uses
#define HM_PLATFORM_UNKNOWN            0
#define HM_PLATFORM_IOS                1
#define HM_PLATFORM_ANDROID            2
#define HM_PLATFORM_WIN32              3
#define HM_PLATFORM_MAC                4
#define HM_PLATFORM_LINUX              5

// Determine target platform by compile environment macro
#define HM_TARGET_PLATFORM             HM_PLATFORM_UNKNOWN

// Apple: Mac and iOS
#if defined(__APPLE__) && !defined(ANDROID) // exclude android for binding generator
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE // TARGET_OS_IPHONE includes TARGET_OS_IOS TARGET_OS_TV and TARGET_OS_WATCH. see TargetConditionals.h
        #undef  HM_TARGET_PLATFORM
        #define HM_TARGET_PLATFORM         HM_PLATFORM_IOS
    #elif TARGET_OS_MAC
        #undef  HM_TARGET_PLATFORM
        #define HM_TARGET_PLATFORM         HM_PLATFORM_MAC
    #endif
#endif

// android
#if defined(ANDROID)
    #undef  HM_TARGET_PLATFORM
    #define HM_TARGET_PLATFORM         HM_PLATFORM_ANDROID
#endif

// win32
#if defined(_WIN32) && defined(_WINDOWS)
    #undef  HM_TARGET_PLATFORM
    #define HM_TARGET_PLATFORM         HM_PLATFORM_WIN32
#endif

// linux
#if defined(LINUX) && !defined(__APPLE__)
    #undef  HM_TARGET_PLATFORM
    #define HM_TARGET_PLATFORM         HM_PLATFORM_LINUX
#endif

// post platform configure

// check user set platform
#if !HM_TARGET_PLATFORM
    #error "Cannot recognize the target platform; are you targeting an unsupported platform?"
#endif

#if (HM_TARGET_PLATFORM == HM_PLATFORM_WIN32)
#ifndef __MINGW32__
#pragma warning (disable:4127)
#endif
#endif  // HM_PLATFORM_WIN32

#if ((HM_TARGET_PLATFORM == HM_PLATFORM_ANDROID) || (HM_TARGET_PLATFORM == HM_PLATFORM_IOS))
    #define HM_PLATFORM_MOBILE
#else
    #define HM_PLATFORM_DESKTOP
#endif


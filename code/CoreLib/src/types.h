#pragma once

#ifdef WIN32
    #include <vectormath/scalar/cpp/vectormath_aos.h>
    #include <stdio.h>
    #include <Windows.h>
    
    #ifdef _MSC_VER 
        #define strncasecmp _strnicmp
        #define strcasecmp _stricmp
    #endif

    typedef Vectormath::Aos::Matrix3 m33;
    typedef Vectormath::Aos::Quat q4;

    typedef Vectormath::Aos::Vector3 v3;
    typedef Vectormath::Aos::Vector4 v4;
    typedef Vectormath::Aos::Matrix4 m44;
#endif

#ifdef PS3
    #include <sys/types.h>
    #include <sys/integertypes.h>
    #include <vectormath/cpp/vectormath_aos.h>
    #include <Ib/Printf.h>

    typedef Vectormath::Aos::Matrix3 m33;
    typedef Vectormath::Aos::Quat q4;

    typedef Vectormath::Aos::Vector3 v3;
    typedef Vectormath::Aos::Vector4 v4;
    typedef Vectormath::Aos::Matrix4 m44;
#endif

#ifdef VITA
    // #define __ARM_NEON__ (1)
    // #define __GNUC__ (1)

    #include <vectormath.h>

    typedef sce::Vectormath::Simd::Aos::Matrix3 m33;
    typedef sce::Vectormath::Simd::Aos::Quat q4;

    typedef sce::Vectormath::Simd::Aos::Vector3 v3;
    typedef sce::Vectormath::Simd::Aos::Vector4 v4;
    typedef sce::Vectormath::Simd::Aos::Matrix4 m44;
#endif

#include <cstddef>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>
#include <string.h>
#include <utility>

#include <Ib/Hook.h>
#include <Ib/Emu.h>
#include <Ib/Memory.h>

#define ARRAY_LENGTH(x) sizeof(x) / sizeof(x[0])
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MIN_MACRO(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MAX_MACRO(a,b) ((a)>(b)?(a):(b))

typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

typedef u16 tchar_t;
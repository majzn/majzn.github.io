#ifndef SAL_H
#define SAL_H

#ifdef __linux__
#define SAL_PLATFORM_LINUX
#elif _WIN32
#define SAL_PLATFORM_WIN32
#endif

/*** STANDARD INCLUDES ***/

#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*** STANDARD TYPES ***/

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed __int64 int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned __int64 uint64;
typedef float real32;
typedef double real64;

typedef int32 SAL_RETURN;
typedef void* SAL_HANDLE;

/*** DEBUGGING & LOGGING ***/

#ifdef SAL_NO_ASSERT
#define SAL_ASSERT(__expr) ((void)0)
#else
#define SAL_ASSERT(__expr)                                  \
  ((__expr) ? (void)0                                       \
            : (fprintf(                                     \
                   stderr,                                  \
                   "assert failed: %s, file %s, line %d\n", \
                   #__expr,                                 \
                   __FILE__,                                \
                   __LINE__),                               \
               abort()))
#endif

/*** STANDARD UTILITIES & MACROS ***/

#undef TRUE
#undef FALSE

#define TRUE  1
#define FALSE 0

#define SAL_ERROR -1

#define SAL_API static

#define SAL_PI  3.14159265358979323846
#define SAL_TAU 6.2831853071795862
#define SAL_EU  2.7182818284590452

#define SAL_EPSILON_32 1e-6
#define SAL_EPSILON_64 1e-9

#define SAL_INT8_MIN  -128
#define SAL_INT8_MAX  127
#define SAL_UINT8_MAX 255

#define SAL_INT16_MIN  -32768
#define SAL_INT16_MAX  32767
#define SAL_UINT16_MAX 65535

#define SAL_INT32_MIN  -2147483648
#define SAL_INT32_MAX  2147483647
#define SAL_UINT32_MAX 4294967295U

#define SAL_INT64_MIN  -9223372036854775808LL
#define SAL_INT64_MAX  9223372036854775807LL
#define SAL_UINT64_MAX 18446744073709551615ULL

#define SAL_REAL32_MIN 1.17549435e-38F
#define SAL_REAL32_MAX 3.40282347e+38F

#define SAL_REAL64_MIN 2.2250738585072014e-308
#define SAL_REAL64_MAX 1.7976931348623157e+308

#define SAL_ABS(__x)                 ((__x) < 0 ? -(__x) : (__x))
#define SAL_SIGN(__a)                ((__a) > 0 ? 1 : ((__a) < 0 ? -1 : 0))
#define SAL_MIN(__a, __b)            ((__a) < (__b) ? (__a) : (__b))
#define SAL_MAX(__a, __b)            ((__a) > (__b) ? (__a) : (__b))
#define SAL_CLAMP(__v, __min, __max) SAL_MIN(SAL_MAX(__v, __min), __max)
#define SAL_SWAP(__x, __y, __temp) \
  {                                \
    __temp = __x;                  \
    __x = __y;                     \
    __y = __temp;                  \
  }

#define SAL_RADIANS(__d) ((__d) * (SAL_PI / 180.0))
#define SAL_DEGREES(__r) ((__r) * (180.0 / SAL_PI))

#define SAL_REAL32_EQ(__a, __b) (fabs((real64)(__a) - (__b)) < SAL_EPSILON_32)
#define SAL_REAL64_EQ(__a, __b) (fabs((__a) - (__b)) < SAL_EPSILON_64)

#define SAL_RGBA(__r, __g, __b, __a) \
  (uint32)(                          \
      (((uint8)(__a)) << 24) |       \
      (((uint8)((__r)) << 16) | (((uint8)(__g)) << 8) | (((uint8)(__b)))))

#define SAL_KB_SIZE 1024UL
#define SAL_MB_SIZE (1024UL * 1024UL)
#define SAL_GB_SIZE (1024UL * 1024UL * 1024UL)

#define SAL_KB(n) ((n) * SAL_KB_SIZE)
#define SAL_MB(n) ((n) * SAL_MB_SIZE)
#define SAL_GB(n) ((n) * SAL_GB_SIZE)

#define SAL_COL_SET_A(__c, __a) (((__c) & 0x00FFFFFF) | ((__a) << 24))
#define SAL_COL_SET_R(__c, __r) (((__c) & 0xFF00FFFF) | ((__r) << 0))
#define SAL_COL_SET_G(__c, __g) (((__c) & 0xFFFF00FF) | ((__g) << 8))
#define SAL_COL_SET_B(__c, __b) (((__c) & 0xFFFFFF00) | ((__b) << 16))
#define SAL_COL_GET_A(__c)      (((__c) >> 24) & 0xFF)
#define SAL_COL_GET_R(__c)      (((__c) >> 16) & 0xFF)
#define SAL_COL_GET_G(__c)      (((__c) >> 8) & 0xFF)
#define SAL_COL_GET_B(__c)      (((__c) >> 0) & 0xFF)

/*** CORE DEFINITIONS ***/

/*** PLATFORM INCLUDES ***/

#ifdef SAL_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef SAL_PLATFORM_LINUX
#endif

/*** PLATFORM LAYER ***/

typedef struct salPlatformContext_t {
} salPlatformContext;

SAL_API int32 sal_sys_init(void);

/*** LIBRARY ***/

/*** USER API ***/

#endif

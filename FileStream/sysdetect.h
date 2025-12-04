#ifndef SYSDETECT_H
#define SYSDETECT_H

#include <cstdint>

// 检测系统位数
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__) || defined(__mips64)
#define SYSTEM_BITS 64
#elif defined(_WIN32) || defined(__i386__) || defined(__ppc__) || defined(__arm__)
#define SYSTEM_BITS 32
#else
#define SYSTEM_BITS_UNKNOWN
#endif

// 检测字节序
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define IS_BIG_ENDIAN 1
#define IS_LITTLE_ENDIAN 0
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IS_BIG_ENDIAN 0
#define IS_LITTLE_ENDIAN 1
#elif defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64) || defined(_M_ARM))
// Windows平台上的小端序
#define IS_BIG_ENDIAN 0
#define IS_LITTLE_ENDIAN 1
#else
// 运行时检测字节序
static inline int is_little_endian()
{
    volatile union
    {
        uint32_t i;
        uint8_t c[4];
    } u;
    u.i = 0x01020304;
    return u.c[0] == 0x04;
}

#define IS_BIG_ENDIAN (!is_little_endian())
#define IS_LITTLE_ENDIAN is_little_endian()
#endif

// 提供统一接口获取系统信息
#ifdef SYSTEM_BITS
#define GET_SYSTEM_BITS() SYSTEM_BITS
#else
#define GET_SYSTEM_BITS() (sizeof(void *) * 8)
#endif

#define IS_SYSTEM_64BIT() (GET_SYSTEM_BITS() == 64)
#define IS_SYSTEM_32BIT() (GET_SYSTEM_BITS() == 32)

#endif // SYSDETECT_H
/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Centralises intrinsic includes and implements standard C fallbacks or inline asm when needed.
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_INTRINSICS_H
#define BASE_INTRINSICS_H

#include "config.h"
#include "macros.h"
#include <stdint.h>

#ifdef HAVE_INTRIN_H
#include <intrin.h>
#endif

#ifdef HAVE_X86INTRIN_H
#include <x86intrin.h>
#endif

#ifdef HAVE_CPUID_H
#include <cpuid.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Temp macro for intrinsic inlining */
#if defined __GNUC__ || defined __clang__
#define __intrin_inline extern __attribute__((gnu_inline, always_inline)) inline
#else
#define __intrin_inline inline
#endif

/* CPUID instructions. */
#ifdef HAVE__CPUID_COUNT
#define __cpuidex(regs, cpuid_type, count) __cpuid_count(cpuid_type, count, regs[0], regs[1], regs[2], regs[3])
#define HAVE_CPUIDEX
#endif

/* GCC and MSVC use the same name but have different signatures so we use a new common name*/
#ifdef HAVE__CPUID_MSVC
#define __cpuidc __cpuid
#define HAVE__CPUID
#elif defined HAVE__CPUID_GCC
#define __cpuidc(regs, cpuid_type) __cpuid(cpuid_type, regs[0], regs[1], regs[2], regs[3])
#define HAVE__CPUID
#endif

/* Intrinsics provided by other compilers on x86, but not watcom */
#ifdef __WATCOMC__
unsigned int __readeflags(void);
#pragma aux __readeflags = \
    "pushfd" \
    "pop eax" value[eax] modify[eax];
#define HAVE__READEFLAGS

unsigned int __writeeflags(unsigned int eflg);
#pragma aux __writeeflags = \
    "push eax" \
    "popfd" parm[eax] modify[];
#define HAVE__WRITEEFLAGS

void __cpuid(int cpuinfo[4], int function_id);
#pragma aux __cpuid = \
    ".586" \
    "cpuid" \
    "mov [esi+0],eax" \
    "mov [esi+4],ebx" \
    "mov [esi+8],ecx" \
    "mov [esi+12],edx" parm[esi][eax] modify[ebx ecx edx];
#define HAVE__CPUID_MSVC

void __cpuidex(int cpuinfo[4], int function_id, int subfunction_id);
#pragma aux __cpuidex = \
    ".586" \
    "cpuid" \
    "mov [esi+0],eax" \
    "mov [esi+4],ebx" \
    "mov [esi+8],ecx" \
    "mov [esi+12],edx" parm[esi][eax][ecx] modify[ebx ecx edx];
#define HAVE__CPUIDEX

unsigned long long __rdtsc(void);
#pragma aux __rdtsc = \
    " .586 " \
    "rdtsc" value[edx eax] modify[] nomemory;
#define HAVE__RDTSC

int _interlockedbittestandset(volatile long *base, long offset);
#pragma aux _interlockedbittestandset = \
    " .586 " \
    "lock bts [eax],edx" \
    "setb cl" \
    "movzx eax,cl" parm[eax edx] value[eax] modify[cl]

extern unsigned short _byteswap_ushort(unsigned short input);
#pragma aux _byteswap_ushort = "xchg al, ah" parm[ax] value[ax] modify[ax]

extern unsigned long _byteswap_ulong(unsigned long input);
#pragma aux _byteswap_ulong = "bswap eax" parm[eax] value[eax] modify[eax]

uint8_t _rotl8(uint8_t a, unsigned b);
#pragma aux _rotl8 = \
    "rol al, cl" parm[al][ecx] value[al] modify[ecx]

uint8_t _rotr8(uint8_t a, unsigned b);
#pragma aux _rotr8 = \
    "ror al, cl" parm[al][ecx] value[al] modify[ecx]

uint16_t _rotl16(uint16_t a, unsigned b);
#pragma aux _rotl16 = \
    "rol ax, cl" parm[ax][ecx] value[ax] modify[ecx]

uint16_t _rotr16(uint16_t a, unsigned b);
#pragma aux _rotr16 = \
    "ror ax, cl" parm[ax][ecx] value[ax] modify[ecx]

uint32_t _rotl(uint32_t a, unsigned b);
#pragma aux _rotl = \
    "rol eax, cl" parm[eax][ecx] value[eax] modify[ecx]

uint32_t _rotr(uint32_t a, unsigned b);
#pragma aux _rotr = \
    "ror eax, cl" parm[eax][ecx] value[eax] modify[ecx]
#define HAVE_MSVC_ROTATE

#endif /* __WATCOMC__ */

/* If we don't have __debugbreak, check for equivalent builtins and finally roll our own if not found. */
#ifndef HAVE__DEBUGBREAK
/* Check for clang intrinsic. */
#if __has_builtin(__builtin_debugtrap)
#define __debugbreak __builtin_debugtrap

/* If we have GCC or compiler that tries to be compatible, use GCC inline assembly. */
#elif defined __GNUC__ || defined __clang__
#if defined(__i386__) || defined(__x86_64__)
__intrin_inline void __debugbreak(void)
{
    __asm__ volatile("int $0x03");
}
#elif defined(__arm__)
__intrin_inline void __debugbreak(void)
{
    __asm__ volatile("bkpt #3");
}
#elif defined(__aarch64__)
__intrin_inline void __debugbreak(void)
{
    /* same values as used by msvc __debugbreak on arm64 */
    __asm__ volatile("brk #0xF000");
}
#elif defined(__powerpc__)
__intrin_inline void __debugbreak(void)
{
    __asm__ volatile(".4byte 0x7d821008");
}
#else
#error __debugbreak not currently supported on this processor platform, see base/intrinsics.h
#endif /* CPU architectures on GCC like compilers */
#elif defined __WATCOMC__
void __debugbreak(void);
#pragma aux __debugbreak = "int 3"
#else
#error __debugbreak not currently supported on this compiler, see base/intrinsics.h
#endif /* compiler specific defines */
#endif /* HAVE__DEBUGBREAK */

/* Rotate instructions */
#ifdef HAVE_GCC_ROTATE
#define __rotl8 __rolb
#define __rotl16 __rolw
#define __rotl32 __rold
#define __rotr8 __rorb
#define __rotr16 __rorw
#define __rotr32 __rord
/* GCC only has the 64 bit intrinsics when building 64bit */
#ifdef __x86_64__
#define __rotl64 __rolq
#define __rotr64 __rorq
#else
#define NEED_64BIT_ROTATE
#endif
#elif defined HAVE_MSVC_ROTATE
#define __rotl8 _rotl8
#define __rotl16 _rotl16
#define __rotr8 _rotr8
#define __rotr16 _rotr16
#define __rotl32 _rotl
#define __rotr32 _rotr
#ifdef __WATCOMC__
#define NEED_64BIT_ROTATE
#else
#define __rotl64 _rotr64
#define __rotr64 _rotr64
#endif
#else
/* These are fallbacks that should generate the correct instructions where available */
__intrin_inline uint8_t __rotl8(uint8_t a, unsigned b)
{
    b &= 7;
    return (a << b) | (a >> (8 - b));
}

__intrin_inline uint8_t __rotr8(uint8_t a, unsigned b)
{
    b &= 7;
    return (a >> b) | (a << (8 - b));
}

__intrin_inline uint16_t __rotl16(uint16_t a, unsigned b)
{
    b &= 15;
    return (a << b) | (a >> (16 - b));
}

__intrin_inline uint16_t __rotr16(uint16_t a, unsigned b)
{
    b &= 15;
    return (a >> b) | (a << (16 - b));
}

__intrin_inline uint32_t __rotl32(uint32_t a, unsigned b)
{
    b &= 31;
    return (a << b) | (a >> (32 - b));
}

__intrin_inline uint32_t __rotr32(uint32_t a, unsigned b)
{
    b &= 31;
    return (a >> b) | (a << (32 - b));
}
#define NEED_64BIT_ROTATE
#endif

#ifdef NEED_64BIT_ROTATE
__intrin_inline uint64_t __rotl64(uint64_t a, unsigned b)
{
    b &= 63;
    return (a << b) | (a >> (64 - b));
}

__intrin_inline uint64_t __rotr64(uint64_t a, unsigned b)
{
    b &= 63;
    return (a >> b) | (a << (64 - b));
}
#undef NEED_64BIT_ROTATE
#endif

#undef __intrin_inline

#ifdef __cplusplus
}
#endif

#endif // BASE_INTRINSICS_H

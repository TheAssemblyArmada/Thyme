/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Centralises intrinsic includes and implements any needed MSVC one in other compilers.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "config.h"
#include "macros.h"

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

// If we don't have __debugbreak, check for equivalent builtins and finally roll our own if not found.
#ifndef HAVE__DEBUGBREAK
// Check for clang intrinsic.
#if __has_builtin(__builtin_debugtrap)
#define __debugbreak __builtin_debugtrap

// If we have GCC or compiler that tries to be compatible, use GCC inline assembly.
#elif defined __GNUC__ || defined __clang__
#if defined(__i386__) || defined(__x86_64__)
extern __attribute__((gnu_inline, always_inline)) inline void __debugbreak(void)
{
    __asm__ volatile("int $0x03");
}
#elif defined(__arm__)
extern __attribute__((gnu_inline, always_inline)) inline void __debugbreak(void)
{
    __asm__ volatile("bkpt #3");
}
#elif defined(__aarch64__)
extern __attribute__((gnu_inline, always_inline)) inline void __debugbreak(void)
{
    // same values as used by msvc __debugbreak on arm64
    __asm__ volatile("brk #0xF000");
}
#elif defined(__powerpc__)
extern __attribute__((gnu_inline, always_inline)) inline void __debugbreak(void)
{
    __asm__ volatile(".4byte 0x7d821008");
}
#else
#error __debugbreak not currently supported on this processor platform, see base/intrinsics.h
#endif // CPU architectures on GCC like compilers
#else
#error __debugbreak not currently supported on this compiler, see base/intrinsics.h
#endif // compiler defines
#endif // HAVE__DEBUGBREAK

#ifdef __cplusplus
}
#endif

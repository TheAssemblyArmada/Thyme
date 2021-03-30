/**
 * @file
 *
 * @author Paul Pedriana (see below)
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Basic header files and defines that are always needed.
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 *
 * This is a heavily modified version of the EACOMPLIER.H file from the EASTL
 * (Electronic Arts Standard Type Library), written by Paul Pedriana of Maxis.
 * Source: http://gpl.ea.com/
 */
#ifndef BASE_PLATFORM_H
#define BASE_PLATFORM_H

/*-----------------------------------------------------------------------------
 * Currently supported platform indentification defines include:
 *    PLATFORM_PS3
 *    PLATFORM_PS3_PPU
 *    PLATFORM_PS3_SPU
 *    PLATFORM_XENON (a.k.a. XBox2)
 *    PLATFORM_ANDROID
 *    PLATFORM_PALM
 *    PLATFORM_FREEBSD
 *    PLATFORM_MAC
 *    PLATFORM_OSX
 *    PLATFORM_LINUX
 *    PLATFORM_WINDOWS
 *    PLATFORM_WIN16
 *    PLATFORM_WIN32
 *    PLATFORM_WIN64
 *    PLATFORM_HPUX
 *    PLATFORM_SUN
 *    PLATFORM_LRB (Larrabee)
 *    PLATFORM_UNIX      (pseudo-platform; may be defined along with another platform like PLATFORM_LINUX)
 *    PLATFORM_CYGWIN    (pseudo-platform; may be defined along with another platform like PLATFORM_LINUX)
 *    PLATFORM_MINGW     (pseudo-platform; may be defined along with another platform like PLATFORM_WINDOWS)
 *    PLATFORM_MICROSOFT (pseudo-platform; may be defined along with another platform like PLATFORM_WINDOWS)
 *
 * Other definitions emanated from this file inclue:
 *    PLATFORM_NAME = <string>
 *    PLATFORM_DESCRIPTION = <string>
 *    PROCESSOR_XXX
 *    SYSTEM_LITTLE_ENDIAN | SYSTEM_BIG_ENDIAN
 *    ASM_STYLE_ATT | ASM_STYLE_INTEL | ASM_STYLE_MOTOROLA
 *    PLATFORM_PTR_SIZE = <integer size in bytes>
 *    PLATFORM_WORD_SIZE = <integer size in bytes>
 *
 *---------------------------------------------------------------------------*/

// Cygwin
// This is a pseudo-platform which will be defined along with PLATFORM_LINUX when
// using the Cygwin build environment.
#if defined(__CYGWIN__)
#define PLATFORM_CYGWIN
#define PLATFORM_DESKTOP
#endif

// MinGW
// This is a pseudo-platform which will be defined along with PLATFORM_WINDOWS when
// using the MinGW Windows build environment.
#if defined(__MINGW32__) || defined(__MINGW64__)
#define PLATFORM_MINGW
#define PLATFORM_DESKTOP
#endif

// PlayStation 3 PPU (Primary Processing Unit)
#if defined(PLATFORM_PS3_PPU) || defined(PLATFORM_PS3) || defined(__PU__) || defined(__PPU__)
#undef PLATFORM_PS3_PPU
#define PLATFORM_PS3_PPU 1
#undef PLATFORM_PS3
#define PLATFORM_PS3 1
#define PLATFORM_NAME "PS3"
#define PROCESSOR_POWERPC
#define PROCESSOR_POWERPC_64
#define SYSTEM_BIG_ENDIAN
#define PLATFORM_DESCRIPTION "PS3 on PowerPC"
#define PLATFORM_CONSOLE

// PlayStation 3 SPU (Synergistic Processing Unit)
#elif defined(PLATFORM_PS3_SPU) || defined(__SPU__)
#undef PLATFORM_PS3_SPU
#define PLATFORM_PS3_SPU 1
#define PLATFORM_NAME "PS3 SPU"
#define PROCESSOR_SPU
#define SYSTEM_BIG_ENDIAN
#define PLATFORM_DESCRIPTION "PS3 SPU on SPU"
#define PLATFORM_CONSOLE

// XBox
// _XBOX is defined by the VC++ project, not the compiler. There is no way
// to tell if the compiler is compiling for XBox unless _XBOX is #defined
// in the project files or otherwise. _M_IX86 is the VC++ way of detecting
// an x86 target, which would mean XBox and not Xenon (a.k.a. XBox2).
#elif defined(PLATFORM_XBOX) || (defined(_XBOX) && defined(_M_IX86))
#undef PLATFORM_XBOX
#define PLATFORM_XBOX 1
#define PLATFORM_NAME "XBox"
#define PROCESSOR_X86
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "XBox on X86"
#if defined(_MSC_VER) || defined(__ICL)
#define ASM_STYLE_INTEL
#endif
#define PLATFORM_CONSOLE

// Xenon (XBox 360)
// The Xenon compiler doesn't define anything in particular to indicate that the
// target is the Xenon platform. The Xenon SDK, however, expects that XBOX and
// _XBOX are #defined, so the project build file must make sure these are defined.
// Since the Xenon compiler in fact defines _M_PPC, we can use this information
// to infer that Xenon is the target if neither _XENON nor _XBOX2 are specifically
// defined by the project build file.
#elif defined(PLATFORM_XENON) || defined(_XENON) || defined(_XBOX2) || ((defined(_XBOX) || defined(XBOX)) && defined(_M_PPC))
#undef PLATFORM_XENON
#define PLATFORM_XENON 1
#define PLATFORM_NAME "Xenon"
#define PROCESSOR_POWERPC
#define PROCESSOR_POWERPC_64
#define SYSTEM_BIG_ENDIAN
#define PLATFORM_DESCRIPTION "Xenon on PowerPC"
#if defined(_MSC_VER) || defined(__ICL)
#define ASM_STYLE_INTEL
#endif
#define PLATFORM_CONSOLE
#define PLATFORM_MICROSOFT 1

// Larrabee
// This part to be removed once __LRB__ is supported by the Larrabee compiler in 2009.
#elif defined(PLATFORM_LRB) || defined(__LRB__) || (defined(__EDG__) && defined(__ICC) && defined(__x86_64__))
#undef PLATFORM_LRB
#define PLATFORM_LRB 1
#define PLATFORM_NAME "Larrabee"
#define PLATFORM_DESCRIPTION "Larrabee on LRB1"
#define PROCESSOR_X86_64
#if defined(BYTE_ORDER) && (BYTE_ORDER == 4321)
#define SYSTEM_BIG_ENDIAN
#else
#define SYSTEM_LITTLE_ENDIAN
#endif
#define PROCESSOR_LRB
#define PROCESSOR_LRB1 // Larrabee version 1
#define ASM_STYLE_ATT // Both types of asm style
#define ASM_STYLE_INTEL // are supported.
#define PLATFORM_DESKTOP

// Android (Google Phone OS)
#elif defined(PLATFORM_ANDROID) || defined(__ANDROID__)
#undef PLATFORM_ANDROID
#define PLATFORM_ANDROID 1
#define PLATFORM_LINUX 1
#define PLATFORM_UNIX 1
#define PLATFORM_NAME "Android"
#define ASM_STYLE_ATT
#if defined(__arm__)
#define PROCESSOR_ARM
#define PLATFORM_DESCRIPTION "Android on ARM"
#elif defined(__aarch64__)
#define PROCESSOR_ARM64
#define PLATFORM_DESCRIPTION "Android on ARM64"
#else
#error Unknown processor
#endif
#if !defined(SYSTEM_BIG_ENDIAN) && !defined(SYSTEM_LITTLE_ENDIAN)
#define SYSTEM_LITTLE_ENDIAN
#endif
#define PLATFORM_MOBILE

// Palm OS for Mobile (Linux variant)
#elif defined(PLATFORM_PALM)
#undef PLATFORM_PALM
#define PLATFORM_PALM 1
#define PLATFORM_LINUX 1
#define PLATFORM_UNIX 1
#define PLATFORM_NAME "Palm"
#define POSIX_THREADS_AVAILABLE 1
#define ASM_STYLE_ATT
#if defined(__arm__)
#define PROCESSOR_ARM
#define PLATFORM_DESCRIPTION "Palm on ARM"
#else
#error Unknown processor
#endif
#if !defined(SYSTEM_BIG_ENDIAN) && !defined(SYSTEM_LITTLE_ENDIAN)
#define SYSTEM_LITTLE_ENDIAN
#endif
#define PLATFORM_MOBILE

// Airplay
#elif defined(PLATFORM_AIRPLAY) || defined(__S3E__)
#undef PLATFORM_AIRPLAY
#define PLATFORM_AIRPLAY
#define PLATFORM_NAME "Airplay"
#if defined(__arm__)
#define PROCESSOR_ARM
#define PLATFORM_DESCRIPTION "Airplay on ARM"
#define ASM_STYLE_ATT
#undef _MSC_VER
#elif defined(_M_IX86)
#define PROCESSOR_X86
#define PLATFORM_DESCRIPTION "Airplay on x86"
#define ASM_STYLE_INTEL
#else
#error Unknown processor
#endif
#if !defined(SYSTEM_BIG_ENDIAN) && !defined(SYSTEM_LITTLE_ENDIAN)
#if defined(HAVE_BIG_ENDIAN) || (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN))
#define SYSTEM_BIG_ENDIAN
#else
#define SYSTEM_LITTLE_ENDIAN
#endif
#endif
#define PLATFORM_MOBILE

// Samsung Bada OS for Mobile (Linux variant)
#elif defined(PLATFORM_BADA)
#undef PLATFORM_BADA
#define PLATFORM_BADA 1
//#define PLATFORM_LINUX 1     // The underlying OS is Linux, but the app mostly doesn't see this.
//#define PLATFORM_UNIX 1
#define PLATFORM_NAME "bada"
#define ASM_STYLE_ATT
#if defined(__arm__)
#define PROCESSOR_ARM
#define PLATFORM_DESCRIPTION "bada on ARM"
#elif defined(__i386__)
#define PLATFORM_BADA_SIMULATOR
#define PROCESSOR_X86
#define PLATFORM_DESCRIPTION "bada simulator on x86"
#else
#error Unknown processor
#endif
#if !defined(SYSTEM_BIG_ENDIAN) && !defined(SYSTEM_LITTLE_ENDIAN)
#define SYSTEM_LITTLE_ENDIAN
#endif
#define PLATFORM_MOBILE

#elif defined(PLATFORM_FREEBSD) || defined(__FreeBSD__)
#undef PLATFORM_FREEBSD
#define PLATFORM_FREEBSD 1
#define PLATFORM_UNIX 1
#define PLATFORM_BSD 1
#define PLATFORM_NAME "FreeBSD"
#if defined(__i386__) || defined(__intel__) || defined(_M_IX86)
#define PROCESSOR_X86
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "FreeBSD on x86"
#elif defined(__x86_64__)
#define PROCESSOR_X86_64
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "FreeBSD on x86-64"
#elif defined(__powerpc64__)
#define PROCESSOR_POWERPC
#define PROCESSOR_POWERPC_64
#define SYSTEM_BIG_ENDIAN
#define PLATFORM_DESCRIPTION "FreeBSD on PowerPC 64"
#elif defined(__powerpc__)
#define PROCESSOR_POWERPC
#define PROCESSOR_POWERPC_32
#define SYSTEM_BIG_ENDIAN
#define PLATFORM_DESCRIPTION "FreeBSD on PowerPC"
#elif defined(__arm__)
#define PROCESSOR_ARM
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "FreeBSD on ARM"
#elif defined(__aarch64__)
#define PROCESSOR_ARM64
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "FreeBSD on ARM64"
#else
#error Unknown processor
#error Unknown endianness
#endif
#if defined(__GNUC__)
#define ASM_STYLE_ATT
#endif
#define PLATFORM_DESKTOP
#elif defined(__APPLE__) && __APPLE__
#include <TargetConditionals.h>

// Apple family of operating systems.
#define PLATFORM_APPLE

// iPhone
// TARGET_OS_IPHONE will be undefined on an unknown compiler, and will be defined on gcc.
#if defined(PLATFORM_IPHONE) || defined(__IPHONE__) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE) \
    || (defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR)
#undef PLATFORM_IPHONE
#define PLATFORM_IPHONE 1
#define PLATFORM_NAME "iPhone"
#define ASM_STYLE_ATT
#define POSIX_THREADS_AVAILABLE 1
#if defined(__arm__)
#define PROCESSOR_ARM
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "iPhone on ARM"
#elif defined(__i386__)
#define PLATFORM_IPHONE_SIMULATOR
#define PROCESSOR_X86
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "iPhone simulator on x86"
#else
#error Unknown processor
#endif
#define PLATFORM_MOBILE

// Macintosh OSX
// TARGET_OS_MAC is defined by the Metrowerks and older AppleC compilers.
// Howerver, TARGET_OS_MAC is defined to be 1 in all cases.
// __i386__ and __intel__ are defined by the GCC compiler.
// __dest_os is defined by the Metrowerks compiler.
// __MACH__ is defined by the Metrowerks and GCC compilers.
// powerc and __powerc are defined by the Metrowerks and GCC compilers.
#elif defined(PLATFORM_OSX) || defined(__MACH__) || (defined(__MSL__) && (__dest_os == __mac_os_x))
#undef PLATFORM_OSX
#define PLATFORM_OSX 1
#define PLATFORM_UNIX 1
#define PLATFORM_NAME "macOS"
#if defined(__i386__) || defined(__intel__)
#define PROCESSOR_X86
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "macOS on x86"
#elif defined(__x86_64) || defined(__amd64)
#define PROCESSOR_X86_64
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "macOS on x86-64"
#elif defined(__arm__)
#define PROCESSOR_ARM
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "macOS on ARM"
#elif defined(__aarch64__)
#define PROCESSOR_ARM64
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "macOS on ARM64"
#elif defined(__POWERPC64__) || defined(__powerpc64__)
#define PROCESSOR_POWERPC
#define PROCESSOR_POWERPC_64
#define SYSTEM_BIG_ENDIAN
#define PLATFORM_DESCRIPTION "OSX on PowerPC 64"
#elif defined(__POWERPC__) || defined(__powerpc__)
#define PROCESSOR_POWERPC
#define PROCESSOR_POWERPC_32
#define SYSTEM_BIG_ENDIAN
#define PLATFORM_DESCRIPTION "OSX on PowerPC"
#else
#error Unknown processor
#endif
#if defined(__GNUC__)
#define ASM_STYLE_ATT
#else
#define ASM_STYLE_MOTOROLA
#endif
#define PLATFORM_DESKTOP

#else
#error Unknown Apple Platform
#endif

// Linux
// __linux and __linux__ are defined by the GCC and Borland compiler.
// __i386__ and __intel__ are defined by the GCC compiler.
// __i386__ is defined by the Metrowerks compiler.
// _M_IX86 is defined by the Borland compiler.
// __sparc__ is defined by the GCC compiler.
// __powerpc__ is defined by the GCC compiler.
#elif defined(PLATFORM_LINUX) || (defined(__linux) || defined(__linux__))
#undef PLATFORM_LINUX
#define PLATFORM_LINUX 1
#define PLATFORM_UNIX 1
#define PLATFORM_NAME "Linux"
#if defined(__i386__) || defined(__intel__) || defined(_M_IX86)
#define PROCESSOR_X86
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "Linux on x86"
#elif defined(__x86_64__)
#define PROCESSOR_X86_64
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "Linux on x86-64"
#elif defined(__powerpc64__)
#define PROCESSOR_POWERPC
#define PROCESSOR_POWERPC_64
#define SYSTEM_BIG_ENDIAN
#define PLATFORM_DESCRIPTION "Linux on PowerPC 64"
#elif defined(__powerpc__)
#define PROCESSOR_POWERPC
#define PROCESSOR_POWERPC_32
#define SYSTEM_BIG_ENDIAN
#define PLATFORM_DESCRIPTION "Linux on PowerPC"
#elif defined(__arm__)
#define PROCESSOR_ARM
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "Linux on ARM"
#elif defined(__aarch64__)
#define PROCESSOR_ARM64
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "Linux on ARM64"
#else
#error Unknown processor
#error Unknown endianness
#endif
#if defined(__GNUC__)
#define ASM_STYLE_ATT
#endif
#define PLATFORM_DESKTOP

// Win CE (Windows mobile)
#elif defined(PLATFORM_WINCE) || defined(_WIN32_WCE)
#undef PLATFORM_WINCE
#define PLATFORM_WINCE 1
#define PLATFORM_NAME "WinCE"
#define ASM_STYLE_INTEL
#define SYSTEM_LITTLE_ENDIAN
#if defined(_M_ARM) // Also there is _M_ARMT
#define PROCESSOR_ARM
#define PLATFORM_DESCRIPTION "Windows CE on ARM"
#elif defined(_M_IX86)
#define PROCESSOR_X86
#define PLATFORM_DESCRIPTION "Windows CE on X86"
#else // Possibly other Windows CE variants
#error Unknown processor
#endif
#define PLATFORM_MOBILE

// Windows
// _WIN16 is defined by the VC++, Intel and GCC compilers.
// _WIN32 is defined by the VC++, Intel and GCC compilers.
// _WIN64 is defined by the VC++, Intel and GCC compilers.
// __WIN32__ is defined by the Borland compiler.
// __INTEL__ is defined by the Metrowerks compiler.
// _M_IX86, _M_AMD64 and _M_IA64 are defined by the VC++, Intel, and Borland compilers.
// _X86_, _AMD64_, and _IA64_ are defined by the Metrowerks compiler.
// _M_ARM is defined by the VC++ compiler.
#elif (defined(PLATFORM_WINDOWS) \
    || (defined(_WIN16) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) \
           || (defined(__MWERKS__) && defined(_X86_)))) \
    && !defined(_XBOX)
#undef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS 1
#define PLATFORM_NAME "Windows"

#ifdef _WIN64 // VC++ defines both _WIN32 and _WIN64 when compiling for Win64.
#define PLATFORM_WIN64
#elif _WIN16 && !_WIN32 // VC++ defines both _WIN16 and _WIN32 when compiling for Win32.
#define PLATFORM_WIN16
#else
#define PLATFORM_WIN32
#endif

#if defined(_M_AMD64) || defined(_AMD64_) || defined(__x86_64__)
#define PROCESSOR_X86_64
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "Windows on X86-64"
#if !defined(_AMD64_)
#define _AMD64_
#endif
#elif defined(_M_IX86) || defined(_X86_)
#define PROCESSOR_X86
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "Windows on X86"
#if !defined(_X86_)
#define _X86_
#endif
#elif defined(_M_IA64) || defined(_IA64_)
#define PROCESSOR_IA64
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "Windows on IA-64"
#elif defined(_M_ARM)
#define PROCESSOR_ARM
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "Windows on ARM"
#if !defined(_ARM_)
#define _ARM_
#endif
#else // Possibly other Windows CE variants
#error Unknown processor
#error Unknown endianness
#endif
#if defined(__GNUC__)
#define ASM_STYLE_ATT
#elif defined(_MSC_VER) || defined(__BORLANDC__) || defined(__ICL)
#define ASM_STYLE_INTEL
#endif
#define PLATFORM_DESKTOP
#define PLATFORM_MICROSOFT 1

// Sun (Solaris)
// __SUNPRO_CC is defined by the Sun compiler.
// __sun is defined by the GCC compiler.
// __i386 is defined by the Sun and GCC compilers.
// __sparc is defined by the Sun and GCC compilers.
#elif defined(PLATFORM_SUN) || (defined(__SUNPRO_CC) || defined(__sun))
#undef PLATFORM_SUN
#define PLATFORM_SUN 1
#define PLATFORM_UNIX 1
#define PLATFORM_NAME "SUN"
#if defined(__i386)
#define PROCESSOR_X86
#define SYSTEM_LITTLE_ENDIAN
#define PLATFORM_DESCRIPTION "SUN on x86"
#elif defined(__sparc)
#define PROCESSOR_SPARC
#define SYSTEM_BIG_ENDIAN
#define PLATFORM_DESCRIPTION "SUN on Sparc"
#else
#error Unknown processor
#error Unknown endianness
#endif
#define PLATFORM_DESKTOP

#else
#error Unknown platform
#error Unknown processor
#error Unknown endianness
#endif

// PLATFORM_PTR_SIZE
// Platform pointer size; same as sizeof(void*).
// This is not the same as sizeof(int), as int is usually 32 bits on
// even 64 bit platforms.
//
// _WIN64 is defined by Win64 compilers, such as VC++.
// _M_IA64 is defined by VC++ and Intel compilers for IA64 processors.
// __LP64__ is defined by HP compilers for the LP64 standard.
// _LP64 is defined by the GCC and Sun compilers for the LP64 standard.
// __ia64__ is defined by the GCC compiler for IA64 processors.
// __arch64__ is defined by the Sparc compiler for 64 bit processors.
// __mips64__ is defined by the GCC compiler for MIPS processors.
// __powerpc64__ is defined by the GCC compiler for PowerPC processors.
// __64BIT__ is defined by the AIX compiler for 64 bit processors.
// __aarch64__ is defined by GCC for ARM64 processors.
// __sizeof_ptr is defined by the ARM compiler (armcc, armcpp).
//
#ifndef PLATFORM_PTR_SIZE
#if defined(__WORDSIZE) // Defined by some variations of GCC.
#define PLATFORM_PTR_SIZE ((__WORDSIZE) / 8)
#elif defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(_M_IA64) || defined(__ia64__) \
    || defined(__arch64__) || defined(__mips64__) || defined(__64BIT__) || defined(__aarch64__) || defined(__x86_64__) \
    || defined(_M_AMD64)
#define PLATFORM_PTR_SIZE 8
#elif defined(__CC_ARM) && (__sizeof_ptr == 8)
#define PLATFORM_PTR_SIZE 8
#else
#define PLATFORM_PTR_SIZE 4
#endif
#endif

// PLATFORM_WORD_SIZE
// This defines the size of a machine word. This will be the same as
// the size of registers on the machine but not necessarily the same
// as the size of pointers on the machine. A number of 64 bit platforms
// have 64 bit registers but 32 bit pointers.
//
#ifndef PLATFORM_WORD_SIZE
#if defined(PLATFORM_XENON) || defined(PLATFORM_PS3)
#define PLATFORM_WORD_SIZE 8
#else
#define PLATFORM_WORD_SIZE PLATFORM_PTR_SIZE
#endif
#endif
#endif // BASE_PLATFORM_H

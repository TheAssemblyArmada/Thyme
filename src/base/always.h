////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: ALWAYS.H
//
//        Author:: CCHyper & OmniBlade
//
//  Contributors:: 
//
//   Description:: Basic header files and defines that are always needed.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _BASE_ALWAYS_H_
#define _BASE_ALWAYS_H_


////////////////////////////////////////////////////////////////////////////////
//
//  Includes
//
////////////////////////////////////////////////////////////////////////////////
#include	"targetver.h"
#include	"config.h"
#include	"macros.h"
#include	"bittype.h"


////////////////////////////////////////////////////////////////////////////////
//
//  External Library Includes
//
////////////////////////////////////////////////////////////////////////////////
#if defined(PLATFORM_WINDOWS)
    #include	<windows.h>
    #include    <sys/utime.h>
    #define 	NAME_MAX 	FILENAME_MAX
#if !defined(PATH_MAX)
    #define 	PATH_MAX 	MAX_PATH
#endif
 
#elif defined(PLATFORM_APPLE) && defined(PLATFORM_OSX)
    #include 	<sys/syslimits.h>
    #include	<unistd.h>
    #include	<strings.h>
    #include    <utime.h>
#elif defined(PLATFORM_LINUX)
    //
    // Force large file support even on 32bit builds.
    //
    #if !defined(_FILE_OFFSET_BITS) || (_FILE_OFFSET_BITS != 64)
        #undef _FILE_OFFSET_BITS
        #define _FILE_OFFSET_BITS 64
    #endif
    #include	<unistd.h>
    #include	<limits.h>
    #include    <utime.h>
#else // !defined(PLATFORM_APPLE) && defined(PLATFORM_OSX)
    #include	<unistd.h>
    #include	<limits.h>
#endif // COMPILER_MSVC || (PLATFORM_APPLE && PLATFORM_OSX)

#include	<sys/types.h>
#include	<sys/stat.h>
#include    <time.h>

////////////////////////////////////////////////////////////////////////////////
//
//  _TIMESPEC_DEFINED.
//    <todo>
//
////////////////////////////////////////////////////////////////////////////////
#if defined(COMPILER_MSVC) && (COMPILER_VERSION >= 1900)
#define _TIMESPEC_DEFINED 1
#endif // COMPILER_MSVC && COMPILER_VERSION >= 1900

////////////////////////////////////////////////////////////////////////////////
//
//  Memory.
//    <todo>
//
////////////////////////////////////////////////////////////////////////////////
//
// Define for debug memory allocation to include __FILE__ and __LINE__ for every memory allocation.
// This helps find leaks.
//
#if defined(GAME_MEMORY_DEBUG)

    #if defined(COMPILER_MSVC)
        #include	<crtdbg.h>
        #include	<stdlib.h>
    #endif // COMPILER_MSVC

    #if defined(COMPILER_MSVC)
        #define		malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
        #define		calloc(c, s)      _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
        #define		realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
        #define		_expand(p, s)     _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
        #define		free(p)           _free_dbg(p, _NORMAL_BLOCK)
        #define		_msize(p)         _msize_dbg(p, _NORMAL_BLOCK)
    #endif // COMPILER_MSVC
#endif // GAME_MEMORY_DEBUG


////////////////////////////////////////////////////////////////////////////////
//
//  General compiler specific
//    <todo>
//
////////////////////////////////////////////////////////////////////////////////
#if defined(COMPILER_MSVC)
    // Allow inline recursive functions within inline recursive functions.
    #pragma inline_recursion(on)

    // Compilers don't necessarily inline code with inline keyword, especially in debug builds.
    // Use FORCE_INLINE to force them to where it is possible.
    //#define FORCE_INLINE	__forceinline
    #define __noinline		__declspec(noinline)
    #define FASTCALL		__fastcall
    #define STDCALL         __stdcall
    #define UNUSED          __pragma(warning(suppress: 4100 4101))
    #define CDECLCALL       __cdecl
    // When including windows, lets just bump the warning level back to 3...
    #pragma warning(push, 3)
    
#else // !COMPILER_MSVC
    //GCC fast call
    #ifdef FASTCALL
        #undef FASTCALL
    #endif
    
    #if defined(COMPILER_GNUC) || defined(COMPILER_CLANG)
        #if defined(PROCESSOR_X86) // Only applies to 32bit
            #define CDECLCALL   __attribute__((cdecl))
            #define FASTCALL	__attribute__((fastcall))
            #define STDCALL	    __attribute__((stdcall))
        #else
            #define CDECLCALL
            #define FASTCALL
            #define STDCALL
        #endif
        
        #define __noinline	    __attribute__((noinline))
        #define UNUSED          __attribute__((unused))

        #ifndef __forceinline
            #define __forceinline	inline __attribute__((__always_inline__))
        #endif
    #else // !COMPILER_GNUC || !COMPILER_CLANG
        //otherwise, nullify fastcall
        #define FASTCALL
        #define CDECLCALL
        #define NO_INLINE
        #define STDCALL
        #ifndef __forceinline
            #define __forceinline	inline
        #endif
    #endif // COMPILER_GNUC || COMPILER_CLANG
    
#endif // COMPILER_MSVC

// Few defines to keep things straight between windows and posix
#if defined(PLATFORM_WINDOWS)
    // Not sure what flags need passing to MinGW-w64 GCC to get wcscasecmp without this
    #define wcscasecmp(a,b)     _wcsicmp(a,b)
#if !defined(PLATFORM_MINGW)
    #define strdup(s)			_strdup(s)
    #define strcasecmp(a,b)		_stricmp(a,b)
    #define strncasecmp(a,b,c)	_strnicmp(a,b,c)
    #define unlink(x)			_unlink(x)
    #define close(x)			_close(x)
#endif

// Moved most stdio stuff to rawfile.cpp as its only used there.
//#if defined(PROCESSOR_X86_64)
    #define utime(x,y)			_utime64(x,y)
    #define stat(x,y)           _stat64(x,y)
    typedef struct _stat64      stat_t;
    typedef struct __utimbuf64  utimbuf_t;
    
#if defined(PLATFORM_WIN32)
    #define ftime(x)			_ftime32(x)
#elif defined(PLATFORM_WIN64)
    #define ftime(x)			_ftime64(x)
#else
    #define ftime(x)			_ftime32(x)

#endif // PLATFORM_WIN32 || PLATFORM_WIN64

// These are apparently implemented correctly in MSVC 2015
#if defined(COMPILER_MSVC) && (COMPILER_VERSION < 1900)
    #define snprintf(...)		_snprintf_s(__VA_ARGS__)
    #define vsnprintf(...)		_vsnprintf_s(__VA_ARGS__)
#endif

    #define localtime_r(a,b)	localtime_s(b,a)

#elif defined(PLATFORM_UNIX)
    typedef struct stat         stat_t;
    typedef struct utimbuf      utimbuf_t;
#endif // COMPILER_MSVC

////////////////////////////////////////////////////////////////////////////////
//
//  Microsoft / Visual Studio
//
////////////////////////////////////////////////////////////////////////////////
// This includes the minimum set of compiler defines and pragmas in order to bring the
// various compilers to a common behavior such that the engine will compile without
// error or warning.
#if defined(COMPILER_MSVC)

    //
    // Turn off some unneeded warnings.
    //

    // It is safe to ignore the following warning from MSVC 7.1 or higher:
    #if (COMPILER_VERSION >= 1310)

        // "new behavior: elements of array will be default initialized"
        #pragma warning(disable: 4351)

    #endif // COMPILER_VERSION >= 1310

    // "unreferenced inline function has been removed" Yea, so what?
    #pragma warning(disable : 4514)

    // "conversion from 'double' to 'float', possible loss of data" Yea, so what?
    #pragma warning(disable : 4244)

    // Disable warning about exception handling not being enabled. It's used as part of STL - in a part of STL we don't use.
    #pragma warning(disable : 4530)

    // "overflow in floating-point constant arithmetic" This warning occurs even if the
    // loss of precision is insignificant.
    #pragma warning(disable : 4056)

    // "function not inlined" This warning is typically useless. The inline keyword
    // only serves as a suggestion to the compiler and it may or may not inline a
    // function on a case by case basis. No need to be told of this.
    #pragma warning(disable : 4710)

    // "'this' used in base member initializer list" Using "this" in a base member
    // initializer is valid -- no need for this warning.
    #pragma warning(disable : 4355)

    // "typedef-name used as a synonym for class-name". This is by design and should
    // not be a warning.
    #pragma warning(disable : 4097)

    // Unreferenced local function removed.
    #pragma warning(disable : 4505)

    // 'function selected for automatic inlining'
    #pragma warning(disable : 4711)
    
    // 'This function or variable may be unsafe. Consider using fopen_s instead. To disable deprecation,
    // use _CRT_SECURE_NO_WARNINGS. See online help for details.'
    #pragma warning(disable : 4996)

    // 'copy constructor could not be generated'
    #pragma warning(disable : 4511)

    // 'assignment operator could not be generated' 
    #pragma warning(disable : 4512)

    // 'unreferenced formal parameter'
    #pragma warning(disable : 4100)

    // HIDE WARNING 4786 "identifier was truncated to '255' characters in the browser information"
    // Tempates create LLLOOONNNGGG identifiers!
    #pragma warning(disable : 4786)

    // 'function selected for automatic inline expansion'.  Cool, but since we're treating
    // warnings as errors, don't warn me about this!
    #pragma warning(disable : 4711)

    // warning C4800: 'BOOL' : forcing value to bool 'true' or 'false' (performance warning)
    #pragma warning(disable : 4800)

    //for catching unrefernced local variables, we do not need to worry about this when compiling debug builds.
#if !defined(NDEBUG)
    #pragma warning(disable : 4189)
    #pragma warning(disable : 4101)
    #pragma warning(disable : 4700)		//uninitialized local variable 'x' used
#endif // !NDEBUG
    
    // To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
    #define _CRT_SECURE_NO_WARNINGS

#endif // COMPILER_MSVC


////////////////////////////////////////////////////////////////////////////////
//
//	Misc
//
////////////////////////////////////////////////////////////////////////////////
//unix and windows handle new lines differently, we handle this here.
#if defined(PLATFORM_WINDOWS)
    #define SYS_NEW_LINE	"\r\n"
    #define SYS_SEPARATOR	'\\'
    #define SYS_NL_SIZE		2
#else // !PLATFORM_WINDOWS
    #define SYS_NEW_LINE	"\n"
    #define SYS_SEPARATOR	'/'
    #define SYS_NL_SIZE		1
#endif // PLATFORM_WINDOWS


////////////////////////////////////////////////////////////////////////////////
//
//  Shorthand cast macros
//
////////////////////////////////////////////////////////////////////////////////
#if !defined(CASTING_MACROS)
    #define		dcast/*<T>(E)*/		dynamic_cast/*<T>(E)*/
    #define		rcast/*<T>(E)*/		reinterpret_cast/*<T>(E)*/
    #define		scast/*<T>(E)*/		static_cast/*<T>(E)*/
    #define		ccast/*<T>(E)*/		const_cast/*<T>(E)*/
    //#define		vcast/*<T>(E)*/		//volatile_cast/*<T>(E)*/
    #define CASTING_MACROS
#endif // !CASTING_MACROS


#endif // _BASE_ALWAYS_H_

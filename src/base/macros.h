////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MACROS.H
//
//        Author:: CCHyper
//
//  Contributors:: OmniBlade
//
//   Description:: 
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
#pragma once

#ifndef BASE_MACROS_H
#define BASE_MACROS_H

////////////////////////////////////////////////////////////////////////////////
//
//  Includes
//
////////////////////////////////////////////////////////////////////////////////
#include	"platform.h"
#include    "compiler.h"


////////////////////////////////////////////////////////////////////////////////
//
//  PACK
//    Some defines for structure packing
//    http://stackoverflow.com/a/3312896
//
//    Example usage:
//        PACK struct _ABC {
//            sint32    A;
//            sint32    B;
//            sint16    C;
//        });
//
////////////////////////////////////////////////////////////////////////////////
#if defined(COMPILER_MSVC)
    #define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
    #pragma warning(disable : 4102) //unreferenced label
    #pragma warning(disable : 4996) //deprecated functions
#elif defined(COMPILER_GNUC) || defined(COMPILER_CLANG)
    #define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif // COMPILER_MSVC || (COMPILER_GNUC || COMPILER_CLANG)


////////////////////////////////////////////////////////////////////////////////
//
//  ARRAY_SIZE
//    Returns the count of items in a built-in C array. This is a common technique
//    which is often used to help properly calculate the number of items in an 
//    array at runtime in order to prevent overruns, etc.
//
//    Example usage:
//        int array[95];
//        size_t arrayCount = ARRAY_SIZE(array); 	// arrayCount is 95.
//
////////////////////////////////////////////////////////////////////////////////
#if !defined(ARRAY_SIZE)
#define	ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif // !ARRAY_SIZE

////////////////////////////////////////////////////////////////////////////////
//
//  SIZE_OF
//    Use the following macro to determine the size of a member:
//
//    Example usage:
//        typedef struct _ABC {
//            sint32    A;
//            sint32    B;
//            sint16    C;
//        } ABC, * PTR_ABC
//    
//        SIZE_OF(struct _ABC, C)
//
////////////////////////////////////////////////////////////////////////////////
#if !defined(SIZE_OF)
#define SIZE_OF(typ,id) sizeof(((typ*)0)->id)
#endif // !SIZE_OF

////////////////////////////////////////////////////////////////////////////////
//
//  ALIGN_OF
//    Determines the alignment of a type.
//
//    Example usage:
//        size_t alignment = ALIGN_OF(int);
//
////////////////////////////////////////////////////////////////////////////////
#if !defined(ALIGN_OF)
    #if defined(COMPILER_METROWERKS)
          #define ALIGN_OF(type) ((size_t)__alignof__(type))
    #elif !defined(COMPILER_GNUC) || (__GNUC__ >= 3) // GCC 2.x doesn't do __alignof correctly all the time.
        #define ALIGN_OF __alignof
    #else
        #define ALIGN_OF(type) ((size_t)OFFSET_OF(struct{ char c; type m; }, m))
    #endif // COMPILER_METROWERKS
#endif // !ALIGN_OF

////////////////////////////////////////////////////////////////////////////////
//
//  DEFINE_ENUMERATION_BITWISE_OPERATORS
//    Define operator overloads to enable bit operations on enum values that are
//    used to define flags. Use DEFINE_ENUMERATION_BITWISE_OPERATORS(ENUMTYPE) to
//    enable these operators on ENUMTYPE.
//
//    Example usage:
//	      typedef enum MyEnum {
//            ENUM_A = 0,
//            ENUM_B = 1,
//            ENUM_C = 2,
//        } MyEnum;
//
//        DEFINE_ENUMERATION_BITWISE_OPERATORS(MyEnum);
//
////////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
    #if !defined(DEFINE_ENUMERATION_BITWISE_OPERATORS)
        #define DEFINE_ENUMERATION_BITWISE_OPERATORS(ENUMTYPE) \
        extern "C++" { \
            __forceinline ENUMTYPE operator | (ENUMTYPE const &a, ENUMTYPE const &b) { return ENUMTYPE(((int &)a) | ((int &)b)); } \
            __forceinline ENUMTYPE operator & (ENUMTYPE const &a, ENUMTYPE const &b) { return ENUMTYPE(((int&)a) & ((int &)b)); } \
            __forceinline ENUMTYPE operator ~ (ENUMTYPE const &a) { return ENUMTYPE(~((int &)a)); } \
            __forceinline ENUMTYPE operator ^ (ENUMTYPE const &a, ENUMTYPE const &b) { return ENUMTYPE(((int &)a) ^ ((int &)b)); } \
            __forceinline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((int &)a) ^= ((int &)b)); } \
            __forceinline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((int &)a) &= ((int &)b)); } \
            __forceinline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((int &)a) |= ((int &)b)); } \
            __forceinline ENUMTYPE operator << (ENUMTYPE &a, int const b) { return (ENUMTYPE)(((int &)a) << ((int)b)); } \
            __forceinline ENUMTYPE operator >> (ENUMTYPE &a, int const b) { return (ENUMTYPE)(((int &)a) >> ((int)b)); } \
            __forceinline ENUMTYPE &operator <<= (ENUMTYPE &a, int const b) { return (ENUMTYPE &)((int &)a = ((int &)a) << ((int)b)); } \
            __forceinline ENUMTYPE &operator >>= (ENUMTYPE &a, int const b) { return (ENUMTYPE &)((int &)a = ((int &)a) >> ((int)b)); } \
        }
    #endif // !DEFINE_ENUMERATION_BITWISE_OPERATORS
#else
    #define DEFINE_ENUMERATION_BITWISE_OPERATORS(ENUMTYPE) // NOP, C allows these operators.
#endif // __cplusplus


////////////////////////////////////////////////////////////////////////////////
//
//  DEFINE_ENUMERATION_OPERATORS
//    Define operator overloads . Use DEFINE_ENUMERATION_OPERATORS(ENUMTYPE)
//    to enable these operators on ENUMTYPE.
//
//    Example usage:
//	      typedef enum MyEnum {
//            ENUM_A = 0,
//            ENUM_B = 1,
//            ENUM_C = 2,
//        } MyEnum;
//
//        DEFINE_ENUMERATION_OPERATORS(MyEnum);
//
////////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
    #if !defined(DEFINE_ENUMERATION_OPERATORS)
        #define DEFINE_ENUMERATION_OPERATORS(ENUMTYPE) \
        extern "C++" { \
            __forceinline ENUMTYPE operator ++ (ENUMTYPE const &a) { return (ENUMTYPE)(++((int &)a)); } \
            __forceinline ENUMTYPE operator -- (ENUMTYPE const &a) { return (ENUMTYPE)(--((int &)a)); } \
            __forceinline ENUMTYPE operator + (ENUMTYPE const &a, ENUMTYPE const &b) { return (ENUMTYPE)(((int &)a) + ((int &)b)); } \
            __forceinline ENUMTYPE operator - (ENUMTYPE const &a, ENUMTYPE const &b) { return (ENUMTYPE)(((int &)a) - ((int &)b)); } \
            __forceinline ENUMTYPE operator * (ENUMTYPE const &a, ENUMTYPE const &b) { return (ENUMTYPE)(((int &)a) * ((int &)b)); } \
            __forceinline ENUMTYPE operator / (ENUMTYPE const &a, ENUMTYPE const &b) { return (ENUMTYPE)(((int &)a) / ((int &)b)); } \
            __forceinline ENUMTYPE operator % (ENUMTYPE const &a, ENUMTYPE const &b) { return (ENUMTYPE)(((int &)a) % ((int &)b)); } \
            __forceinline ENUMTYPE &operator += (ENUMTYPE &a, ENUMTYPE const b) { return (ENUMTYPE &)((int &)a = ((int &)a) + ((ENUMTYPE)b)); } \
            __forceinline ENUMTYPE &operator -= (ENUMTYPE &a, ENUMTYPE const b) { return (ENUMTYPE &)((int &)a = ((int &)a) - ((ENUMTYPE)b)); } \
            __forceinline ENUMTYPE operator ++ (ENUMTYPE const &a, int const b) { return (ENUMTYPE)(++((int &)a)); } \
            __forceinline ENUMTYPE operator -- (ENUMTYPE const &a, int const b) { return (ENUMTYPE)(--((int &)a)); } \
            __forceinline ENUMTYPE operator + (ENUMTYPE const &a, int const b) { return (ENUMTYPE)(((int &)a) + ((int)b)); } \
            __forceinline ENUMTYPE operator - (ENUMTYPE const &a, int const b) { return (ENUMTYPE)(((int &)a) - ((int)b)); } \
            __forceinline ENUMTYPE operator * (ENUMTYPE const &a, int const b) { return (ENUMTYPE)(((int &)a) * ((int)b)); } \
            __forceinline ENUMTYPE operator / (ENUMTYPE const &a, int const b) { return (ENUMTYPE)(((int &)a) / ((int)b)); } \
            __forceinline ENUMTYPE operator % (ENUMTYPE const &a, int const b) { return (ENUMTYPE)(((int &)a) % ((int)b)); } \
            __forceinline ENUMTYPE &operator += (ENUMTYPE &a, int const b) { return (ENUMTYPE &)((int &)a = ((int &)a) + ((int)b)); } \
            __forceinline ENUMTYPE &operator -= (ENUMTYPE &a, int const b) { return (ENUMTYPE &)((int &)a = ((int &)a) - ((int)b)); } \
            /*__forceinline bool operator != (ENUMTYPE const &a, ENUMTYPE const &b) { return (ENUMTYPE)((int &)a != ((int &)b)); }*/ \
            /*__forceinline bool operator == (ENUMTYPE const &a, ENUMTYPE const &b) { return (ENUMTYPE)((int &)a == ((int &)b)); }*/ \
        }
    #endif // !DEFINE_ENUMERATION_OPERATORS
#else
    #define DEFINE_ENUMERATION_OPERATORS(ENUMTYPE) // NOP, C allows these operators.
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////
//
//  __CURRENT_FUNCTION__
//    
//
//    Example usage:
//        
//
////////////////////////////////////////////////////////////////////////////////
#if !defined(__CURRENT_FUNCTION__)
    #if defined (COMPILER_GNUC) || (defined (COMPILER_INTEL) && COMPILER_VERSION >= 600)
        #define __CURRENT_FUNCTION__ __PRETTY_FUNCTION__
    #elif defined(__FUNCSIG__) || defined(COMPILER_MSVC)
        #define __CURRENT_FUNCTION__ __FUNCSIG__
    #elif defined(COMPILER_INTEL)
        #define __CURRENT_FUNCTION__ __FUNCTION__
    #elif defined(COMPILER_BORLANDC)
        #define __CURRENT_FUNCTION__ __FUNC__
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901
        #define __CURRENT_FUNCTION__ __func__
    #else
        #define __CURRENT_FUNCTION__	"(unknown function)"
    #endif
#endif // __CURRENT_FUNCTION__


////////////////////////////////////////////////////////////////////////////////
//
//  COMPILER_MESSAGE, COMPILER_WARNING, COMPILER_ERROR and COMPILER_TODO
//    
//
//    Example usage:
//        COMPILER_MESSAGE("My Message");
//        COMPILER_WARNING("My Message");
//        COMPILER_ERROR("My Message");
//        COMPILER_TODO("My Message");
//
////////////////////////////////////////////////////////////////////////////////
#if !defined(COMPILER_MESSAGE) || !defined(COMPILER_WARNING) || !defined(COMPILER_ERROR) || !defined(COMPILER_TODO)

    // Decide what pragma command to use
    #if defined(COMPILER_CLANG)
        #define		PRAGMA(x)		_Pragma(#x)
    #elif defined(COMPILER_GNUC)
        #define		PRAGMA(x)		_Pragma(#x)
    #elif defined(COMPILER_MSVC)
        #define		PRAGMA(x)		__pragma(x)
    #endif // COMPILER_CLANG || COMPILER_GNUC || COMPILER_MSVC

    // Some helpful macros for formatting strings
    #define		STRINGIZE_HELPER(str)	#str
    #define		STRINGIZE(str)			STRINGIZE_HELPER(str)

    #define		__LINE                  STRINGIZE( __LINE__ )
    #define		__FILE_LINE             __FILE__ "(" __LINE ")"
    #define		__FILE_FUNC_LINE		__FILE__ "(" __CURRENT_FUNCTION__ ")" "(" __LINE ")"

    // The formatter
    #define		INFO(info, msg)			PRAGMA(message( __FILE_FUNC_LINE ": " #info ": " msg ))

    // The message macros
    #if defined(IGNORE_BUILD_MACROS) || defined(NDEBUG)
        #define		COMPILER_MESSAGE(msg)		{}
        #define		COMPILER_WARNING(msg)		{}
        #define		COMPILER_ERROR(msg)		{}
        #define		COMPILER_TODO(msg)			{}
    #else // !IGNORE_BUILD_MACROS || !NDEBUG
        #if defined(COMPILER_MSVC)
            #define		COMPILER_MESSAGE(msg)		INFO(Message, msg)
            #define		COMPILER_WARNING(msg)		INFO(Warning!, msg)
            #define		COMPILER_ERROR(msg)		    INFO(Error!, msg)
            #define		COMPILER_TODO(msg)			INFO(TODO, msg)
        #else
            #define		COMPILER_MESSAGE(msg)		PRAGMA(message("INFO : " #msg))
            #define		COMPILER_WARNING(msg)		PRAGMA(message("WARNING : " #msg))
            #define		COMPILER_ERROR(msg)		PRAGMA(message("ERROR : " #msg))
            #define		COMPILER_TODO(msg)			PRAGMA(message("TODO : " #msg))
        #endif
    #endif // IGNORE_BUILD_MACROS || NDEBUG

#endif // !COMPILER_MESSAGE || !COMPILER_WARNING || !COMPILER_ERROR || !COMPILER_TODO


////////////////////////////////////////////////////////////////////////////////
//
//  INLINE_ASM
//    
//
//    Example usage:
//        INLINE_ASM( fld     dword ptr[x] );
//        INLINE_ASM( fabs                 );
//        INLINE_ASM( fstp    dword ptr[x] );
//
////////////////////////////////////////////////////////////////////////////////
#if !defined(INLINE_ASM)
    #if defined(COMPILER_MSVC)
        #define INLINE_ASM(asm_literal) __asm { asm_literal };
    #elif defined(COMPILER_GNUC) || defined(COMPILER_CLANG)
        #define INLINE_ASM(asm_literal) \
        "__asm__(\".intel_syntax noprefix\");" \
        "__asm__(\"" #asm_literal "\" : : );" \
        "__asm__(\".att_syntax prefix\");"
    #endif // COMPILER_MSVC || COMPILER_GNUC || COMPILER_CLANG
#endif // !INLINE_ASM

#endif // _BASE_MACROS_H_

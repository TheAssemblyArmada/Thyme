////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: RAWALLOC.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Wrapper around platform memory allocators.
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

#ifndef _RAWALLOC_H_
#define _RAWALLOC_H_

#include "always.h"
#include "gamedebug.h"
#include <cstdlib>

// Use GlobalAlloc as the raw allocator on windows to avoid CRT issues.
// Needed until runs standalone then just use malloc/calloc
#ifdef PLATFORM_WINDOWS
inline void *Raw_Allocate(int bytes)
{
    void *r = GlobalAlloc(GMEM_ZEROINIT | GMEM_FIXED, bytes);

    //if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    ASSERT_THROW(r != nullptr, 0xDEAD0002);

    return r;
}

inline void *Raw_Allocate_No_Zero(int bytes)
{
    void *r = GlobalAlloc(GMEM_FIXED, bytes);

    //if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    ASSERT_THROW(r != nullptr, 0xDEAD0002);

    return r;
}

inline void Raw_Free(void *memory) { if ( memory != nullptr ) GlobalFree(memory); }

// Otherwise use standard allocators.
#else
inline void *Raw_Allocate(int bytes)
{
    void *r = calloc(1, bytes);

    //if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    ASSERT_THROW(r != nullptr, 0xDEAD0002);

    return r;
}

inline void *Raw_Allocate_No_Zero(int bytes)
{
    void *r = malloc(bytes);

    //if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    ASSERT_THROW(r != nullptr, 0xDEAD0002);

    return r;
}

inline void Raw_Free(void *memory) { if ( memory != nullptr ) free(memory); }
#endif

inline int Round_Up_4(int number) { return (number + 3) & (~3); }   // For 4byte alignment
inline int Round_Up_8(int number) { return (number + 7) & (~7); }   // For 8bytes alignment
inline int Round_Up_Word_Size(int number) { return (number + sizeof(void*) - 1) & (~(sizeof(void*) - 1)); } // For machine wordsize alignment

#endif // _RAWALLOC_H_

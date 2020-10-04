/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Wrappers around platform memory allocators.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include <captainslog.h>
#include <cstdlib>

// Use GlobalAlloc as the raw allocator on windows to avoid CRT issues.
// Needed until runs standalone then just use malloc/calloc
#ifdef PLATFORM_WINDOWS
#include <winbase.h>

/**
 * @brief Allocates a block of memory, contents will be zeroed.
 */
inline void *Raw_Allocate(int bytes)
{
    void *r = GlobalAlloc(GMEM_ZEROINIT | GMEM_FIXED, bytes);

    // if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    captainslog_relassert(r != nullptr, 0xDEAD0002, "Memory allocation failed.");

    return r;
}

/**
 * @brief Allocates a block of memory, contents will be random.
 */
inline void *Raw_Allocate_No_Zero(int bytes)
{
    void *r = GlobalAlloc(GMEM_FIXED, bytes);

    // if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    captainslog_relassert(r != nullptr, 0xDEAD0002, "Memory allocation failed.");

    return r;
}

/**
 * @brief Frees a block of memory.
 */
inline void Raw_Free(void *memory)
{
    if (memory != nullptr)
        GlobalFree(memory);
}

// Otherwise use standard allocators.
#else
/**
 * @brief Allocates a block of memory, contents will be zeroed.
 */
inline void *Raw_Allocate(int bytes)
{
    void *r = calloc(1, bytes);

    // if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    captainslog_relassert(r != nullptr, 0xDEAD0002, "Memory allocation failed.");

    return r;
}

/**
 * @brief Allocates a block of memory, contents will be random.
 */
inline void *Raw_Allocate_No_Zero(int bytes)
{
    void *r = malloc(bytes);

    // if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    captainslog_relassert(r != nullptr, 0xDEAD0002, "Memory allocation failed.");

    return r;
}

/**
 * @brief Frees a block of memory.
 */
inline void Raw_Free(void *memory)
{
    if (memory != nullptr)
        free(memory);
}
#endif

inline int Round_Up_4(int number)
{
    return (number + 3) & (~3);
} // For 4byte alignment
inline int Round_Up_8(int number)
{
    return (number + 7) & (~7);
} // For 8bytes alignment
/**
 * @brief Rounds a number up to a multiple of the architecture word size.
 */
inline int Round_Up_Word_Size(int number)
{
    return (number + sizeof(void *) - 1) & (~(sizeof(void *) - 1));
} // For machine wordsize alignment
/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Hooks for CRT functions to ensure Thyme calls the same verions as the original for functions that maintain state.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "hooker.h"

// Define references to msvcrt functions in the original binary that maintain state
// Only important if calls relying on state are made by both the original binary
// and Thyme.
#define crt_strtok (Make_Global<char *(__cdecl *const)(char *, const char *)>(0x00939424))
#define crt_free (Make_Global<void(__cdecl *const)(void *)>(0x00939364))
#define crt_malloc (Make_Global<void *(__cdecl *const)(size_t)>(0x0093932C))
#define crt_set_se_translator \
    (Make_Global<void(__cdecl *const)(void(__cdecl *)(unsigned int, struct _EXCEPTION_POINTERS *))>(0x009393A0))

#ifdef malloc
#undef malloc
#endif
#define malloc crt_malloc

#ifdef free
#undef free
#endif
#define free crt_free

#ifdef strtok
#undef strtok
#endif
#define strtok crt_strtok
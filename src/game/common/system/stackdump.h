/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Functions for creating dumps of information on program crash.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "asciistring.h"

#ifdef PLATFORM_WINDOWS
// clang-format off
// Headers must be in this order.
#include <winbase.h>
#include <winver.h>
#include <dbghelp.h>
#include <eh.h>
// clang-format on

void __cdecl Dump_Exception_Info(unsigned int u, struct _EXCEPTION_POINTERS *e_info);
BOOL Init_Symbol_Info();
void Uninit_Symbol_Info();
void __cdecl Stack_Dump_Handler(const char *data);
void Make_Stack_Trace(
    uintptr_t myeip, uintptr_t myesp, uintptr_t myebp, int skipFrames, void(__cdecl *callback)(char const *));
#endif

#ifdef GAME_DLL
extern Utf8String &g_exceptionFileBuffer;
#else
extern Utf8String g_exceptionFileBuffer;
#endif
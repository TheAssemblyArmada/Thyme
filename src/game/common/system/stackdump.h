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
#include <winver.h>
#include <dbghelp.h>
#include <eh.h>

void Dump_Exception_Info(unsigned int u, struct _EXCEPTION_POINTERS *e_info);
BOOL Init_Symbol_Info();
void Uninit_Symbol_Info();
void Stack_Dump_Handler(const char *data);
void Make_Stack_Trace(uintptr_t myeip, uintptr_t myesp, uintptr_t myebp, int skipFrames, void (*callback)(char const *));
#endif

#ifdef GAME_DLL
#include "hooker.h"

extern Utf8String &g_exceptionFileBuffer;
#else
extern Utf8String g_exceptionFileBuffer;
#endif
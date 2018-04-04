/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Custom memory manager designed to limit OS calls to allocate heap memory.
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
#include "gamedebug.h"
#include "rawalloc.h"

class MemoryPool;

extern int g_theLinkChecker;
extern bool g_thePreMainInitFlag;
extern bool g_theMainInitFlag;

//#define TheLinkChecker (Make_Global<int>(0x00A29B9C))
void Init_Memory_Manager();
void Init_Memory_Manager_Pre_Main();
void Shutdown_Memory_Manager();
MemoryPool *Create_Named_Pool(const char *name, int size);

#ifndef THYME_STANDALONE
#include "hooker.h" //Remove once all hooks implemented

void *New_New(size_t bytes);
void New_Delete(void *ptr);

namespace GameMemory
{
inline void Hook_Me()
{
    // Replace memory intialisation
    Hook_Function(0x00414510, Init_Memory_Manager);
    Hook_Function(0x004148C0, Init_Memory_Manager_Pre_Main);

    // Replace memory allocation operators
    Hook_Function(0x00414450, New_New); // operator new
    Hook_Function(0x00414490, New_New); // operator new[]
    Hook_Function(0x004144D0, New_Delete); // operator delete
    Hook_Function(0x004144F0, New_Delete); // operator delete[]
    Hook_Function(0x00414B30, Create_Named_Pool);
}
} // namespace GameMemory

#endif
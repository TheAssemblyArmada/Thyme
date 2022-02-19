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
#include "rawalloc.h"

class MemoryPool;

#ifdef GAME_DLL
extern bool &g_thePreMainInitFlag;
extern bool &g_theMainInitFlag;
#else
extern bool g_thePreMainInitFlag;
extern bool g_theMainInitFlag;
#endif

#ifdef GAME_DEBUG
extern int32_t g_theLinkChecker;
#endif

void Init_Memory_Manager();
void Init_Memory_Manager_Pre_Main();
void Shutdown_Memory_Manager();
MemoryPool *Create_Named_Pool(const char *name, int32_t size);
void *Allocate_From_Pool(MemoryPool *pool, int32_t size);
void Free_From_Pool(MemoryPool *pool, void *memory);

#ifdef GAME_DLL
void *New_New(size_t bytes);
void New_Delete(void *ptr);
#endif

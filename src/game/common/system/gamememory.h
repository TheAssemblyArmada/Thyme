////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEMEMORY.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Custom memory manager designed to limit OS calls to allocate
//                 heap memory.
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

#ifndef GAMEMEMORY_H
#define GAMEMEMORY_H

#include "always.h"
#include "gamedebug.h"
#include "rawalloc.h"
#include "hooker.h"         //Remove once all hooks implemented

void *New_New(size_t bytes);
void *New_Array_New(size_t bytes);
void New_Delete(void *ptr);
void New_Array_Delete(void *ptr);

struct PoolInitRec;

class MemoryPoolFactory;
class MemoryPoolBlob;
class MemoryPoolSingleBlock;
class MemoryPool;
class DynamicMemoryAllocator;
class SimpleCriticalSectionClass;

#define TheLinkChecker (Make_Global<int>(0x00A29B9C))
extern bool ThePreMainInitFlag;
extern bool TheMainInitFlag;

void Init_Memory_Manager();
void Init_Memory_Manager_Pre_Main();
void Shutdown_Memory_Manager();
MemoryPool *Create_Named_Pool(char const *name, int size);

#endif // _GAMEMEMORY_H

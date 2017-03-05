////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEMEMORY.CPP
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
#include "gamememory.h"
#include "gamememoryinit.h"
#include "critsection.h"
#include "gamedebug.h"
#include "memblob.h"
#include "memblock.h"
#include "memdynalloc.h"
#include "mempool.h"
#include "mempoolfact.h"
#include "minmax.h"

//////////
// Globals
//////////

bool ThePreMainInitFlag = false;
bool TheMainInitFlag = false;

///////////////////////////////////
// Memory Manager Control Functions
///////////////////////////////////

void Init_Memory_Manager()
{
    int param_count;
    PoolInitRec const *params;

    if ( TheMemoryPoolFactory == nullptr ) {
        DEBUG_LOG("Memory Manager initialising normally.\n");
        User_Memory_Get_DMA_Params(&param_count, &params);
        TheMemoryPoolFactory = new MemoryPoolFactory;
        TheMemoryPoolFactory->Init();
        TheDynamicMemoryAllocator = TheMemoryPoolFactory->Create_Dynamic_Memory_Allocator(param_count, params);
        User_Memory_Init_Pools();
        ThePreMainInitFlag = false;
    }

    //
    // Check that new and delete both use our custom implementation.
    // 
    TheLinkChecker = 0;

    DEBUG_LOG("Checking memory manager operators are linked, link checker at %d\n", TheLinkChecker);

    char *tmp = new char;
    delete tmp;
    tmp = new char[8];
    delete[] tmp;
    SimpleCriticalSectionClass *tmp2 = new SimpleCriticalSectionClass;
    delete tmp2;

    if ( TheLinkChecker != 6 ) {
        DEBUG_LOG("Not linked correct new and delete operators, checker has value %d\n", TheLinkChecker);
        exit(-1);
    }

    DEBUG_LOG("Memory manager operators passed check, link checker at %d\n", TheLinkChecker);

    TheMainInitFlag = true;
}

void Init_Memory_Manager_Pre_Main()
{
    int param_count;
    PoolInitRec const *params;

    if ( TheMemoryPoolFactory == nullptr ) {
        DEBUG_INIT(DEBUG_LOG_TO_FILE);
        DEBUG_LOG("Memory Manager initialising prior to WinMain\n");

        User_Memory_Get_DMA_Params(&param_count, &params);
        TheMemoryPoolFactory = new MemoryPoolFactory;
        TheMemoryPoolFactory->Init();
        TheDynamicMemoryAllocator = TheMemoryPoolFactory->Create_Dynamic_Memory_Allocator(param_count, params);
        User_Memory_Init_Pools();
        ThePreMainInitFlag = true;
    }
}

void Shutdown_Memory_Manager()
{
    if ( !ThePreMainInitFlag ) {
        if ( TheMemoryPoolFactory != nullptr ) {
            if ( TheDynamicMemoryAllocator != nullptr ) {
                TheMemoryPoolFactory->Destroy_Dynamic_Memory_Allocator(TheDynamicMemoryAllocator);
                TheDynamicMemoryAllocator = nullptr;
            }

            delete TheMemoryPoolFactory;
            TheMemoryPoolFactory = nullptr;
        }
    }

    TheMainInitFlag = false;
}

////////////////////////
// Replacement operators
////////////////////////

MemoryPool *Create_Named_Pool(char const *name, int size)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();
    return TheMemoryPoolFactory->Create_Memory_Pool(name, size, 0, 0);
}

//
// These all override the global news and deletes just by being linked.
//
void *New_New(size_t bytes)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();

    return TheDynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void *operator new(size_t bytes)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();

    return TheDynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void *operator new[](size_t bytes)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();

    return TheDynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void New_Delete(void *ptr)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();
    TheDynamicMemoryAllocator->Free_Bytes(ptr);
}

void operator delete(void *ptr)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();
    TheDynamicMemoryAllocator->Free_Bytes(ptr);
}

void operator delete[](void *ptr)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();
    TheDynamicMemoryAllocator->Free_Bytes(ptr);
}

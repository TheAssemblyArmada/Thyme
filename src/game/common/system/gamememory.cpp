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
#include "gamememory.h"
#include "critsection.h"
#include "gamememoryinit.h"
#include "memblob.h"
#include "memblock.h"
#include "memdynalloc.h"
#include "mempool.h"
#include "mempoolfact.h"

#ifndef GAME_DLL
bool g_thePreMainInitFlag = false;
bool g_theMainInitFlag = false;
#else
#include "hooker.h"
#endif

#ifdef GAME_DEBUG
int g_theLinkChecker = 0;
#endif

#ifdef GAME_DLL
void *New_New(size_t bytes)
{
    ++(Make_Global<int>(PICK_ADDRESS(0x00A29B9C, 0x00E1B368)));
    Init_Memory_Manager_Pre_Main();

    return g_dynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void *New_New_Dbg(size_t bytes, char const *file, int line)
{
    ++(Make_Global<int>(PICK_ADDRESS(0x00A29B9C, 0x00E1B368)));
    Init_Memory_Manager_Pre_Main();

    return g_dynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void New_Delete(void *ptr)
{
    ++(Make_Global<int>(PICK_ADDRESS(0x00A29B9C, 0x00E1B368)));
    Init_Memory_Manager_Pre_Main();
    g_dynamicMemoryAllocator->Free_Bytes(ptr);
}

void New_Delete_Dbg(void *ptr, char const *file, int line)
{
    ++(Make_Global<int>(PICK_ADDRESS(0x00A29B9C, 0x00E1B368)));
    Init_Memory_Manager_Pre_Main();
    g_dynamicMemoryAllocator->Free_Bytes(ptr);
}
#endif

void Init_Memory_Manager()
{
    int param_count;
    PoolInitRec const *params;

    if (g_memoryPoolFactory == nullptr) {
        captainslog_trace("Memory Manager initialising normally.\n");
        User_Memory_Get_DMA_Params(&param_count, &params);
        g_memoryPoolFactory = new MemoryPoolFactory;
        g_memoryPoolFactory->Init();
        g_dynamicMemoryAllocator = g_memoryPoolFactory->Create_Dynamic_Memory_Allocator(param_count, params);
        User_Memory_Init_Pools();
        g_thePreMainInitFlag = false;
    }

#ifdef GAME_DEBUG
    // Check that new and delete both use our custom implementation.
    g_theLinkChecker = 0;

    captainslog_info("Checking memory manager operators are linked, link checker at %d\n", g_theLinkChecker);

    char *tmp = new char;
    delete tmp;
    char *tmp2 = new char[8];
    delete[] tmp2;
    SimpleCriticalSectionClass *tmp3 = new SimpleCriticalSectionClass;
    delete tmp3;

    if (g_theLinkChecker != 6) {
        captainslog_fatal("Not linked correct new and delete operators, checker has value %d\n", g_theLinkChecker);
        exit(-1);
    }
#endif

    g_theMainInitFlag = true;
}

void Init_Memory_Manager_Pre_Main()
{
    int param_count;
    PoolInitRec const *params;

    if (g_memoryPoolFactory == nullptr) {
        captainslog_trace("Memory Manager initialising prior to WinMain\n");

        User_Memory_Get_DMA_Params(&param_count, &params);
        g_memoryPoolFactory = new MemoryPoolFactory;
        g_memoryPoolFactory->Init();
        g_dynamicMemoryAllocator = g_memoryPoolFactory->Create_Dynamic_Memory_Allocator(param_count, params);
        User_Memory_Init_Pools();
        g_thePreMainInitFlag = true;
    }
}

void Shutdown_Memory_Manager()
{
    if (!g_thePreMainInitFlag) {
        if (g_memoryPoolFactory != nullptr) {
            if (g_dynamicMemoryAllocator != nullptr) {
                g_memoryPoolFactory->Destroy_Dynamic_Memory_Allocator(g_dynamicMemoryAllocator);
                g_dynamicMemoryAllocator = nullptr;
            }

            delete g_memoryPoolFactory;
            g_memoryPoolFactory = nullptr;
        }
    }

    g_theMainInitFlag = false;
}

MemoryPool *Create_Named_Pool(const char *name, int size)
{
#ifdef GAME_DEBUG
    ++g_theLinkChecker;
#endif
    Init_Memory_Manager_Pre_Main();
    return g_memoryPoolFactory->Create_Memory_Pool(name, size, 0, 0);
}

void *Allocate_From_Pool(MemoryPool *pool, int size)
{
    return pool->Allocate_Block();
}

void Free_From_Pool(MemoryPool *pool, void *memory)
{
    pool->Free_Block(memory);
}

// These all override the global news and deletes just by being linked.
void *operator new(size_t bytes)
{
#ifdef GAME_DEBUG
    ++g_theLinkChecker;
#endif
    Init_Memory_Manager_Pre_Main();

    return g_dynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void *operator new[](size_t bytes)
{
#ifdef GAME_DEBUG
    ++g_theLinkChecker;
#endif
    Init_Memory_Manager_Pre_Main();

    return g_dynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void operator delete(void *ptr)
{
#ifdef GAME_DEBUG
    ++g_theLinkChecker;
#endif
    Init_Memory_Manager_Pre_Main();
    g_dynamicMemoryAllocator->Free_Bytes(ptr);
}

void operator delete[](void *ptr)
{
#ifdef GAME_DEBUG
    ++g_theLinkChecker;
#endif
    Init_Memory_Manager_Pre_Main();
    g_dynamicMemoryAllocator->Free_Bytes(ptr);
}

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
#include "rawalloc.h"

#pragma warning(push)
#pragma warning(disable : 4073) // warning C4073: initializers put in library initialization area
#pragma init_seg(lib) // Forces objects and variables in this file to initialize before other stuff.
#pragma warning(pop)

#ifndef GAME_DLL
bool g_thePreMainInitFlag = false;
bool g_theMainInitFlag = false;
#else
#include "hooker.h"
#endif

#ifdef GAME_DLL
int &g_theLinkChecker = Make_Global<int>(PICK_ADDRESS(0x00A29B9C, 0x00E1B368));
#else
int g_theLinkChecker = 0;
#endif

#ifdef GAME_DLL
void *New_New(size_t bytes)
{
    ++g_theLinkChecker;
    Init_Memory_Manager_Pre_Main();
    captainslog_dbgassert(g_dynamicMemoryAllocator, "must init memory manager before calling global operator new");

    return g_dynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void *New_New_Dbg(size_t bytes, char const *file, int line)
{
    ++g_theLinkChecker;
    Init_Memory_Manager_Pre_Main();
    captainslog_dbgassert(g_dynamicMemoryAllocator, "must init memory manager before calling global operator new");

    return g_dynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void New_Delete(void *ptr)
{
    ++g_theLinkChecker;
    Init_Memory_Manager_Pre_Main();
    captainslog_dbgassert(g_dynamicMemoryAllocator, "must init memory manager before calling global operator delete");

    g_dynamicMemoryAllocator->Free_Bytes(ptr);
}

void New_Delete_Dbg(void *ptr, char const *file, int line)
{
    ++g_theLinkChecker;
    Init_Memory_Manager_Pre_Main();
    captainslog_dbgassert(g_dynamicMemoryAllocator, "must init memory manager before calling global operator delete");

    g_dynamicMemoryAllocator->Free_Bytes(ptr);
}
#endif

void Init_Memory_Manager()
{
    int param_count;
    PoolInitRec const *params;

    if (g_memoryPoolFactory == nullptr) {
        User_Memory_Get_DMA_Params(&param_count, &params);
        g_memoryPoolFactory = new MemoryPoolFactory;
        g_memoryPoolFactory->Init();
        g_dynamicMemoryAllocator = g_memoryPoolFactory->Create_Dynamic_Memory_Allocator(param_count, params);
        User_Memory_Init_Pools();
        g_thePreMainInitFlag = false;
    } else {
        captainslog_dbgassert(g_thePreMainInitFlag, "memory manager is already inited");
    }

#if defined GAME_DEBUG
    // Check that new and delete both use our custom implementation.
    g_theLinkChecker = 0;

    captainslog_info("Checking memory manager operators are linked, link checker at %d", g_theLinkChecker);

    char *tmp = new char;
    delete tmp;
    char *tmp2 = new char[8];
    delete[] tmp2;
    char *tmp3 = new char;
    *tmp3 = 1;
    delete tmp3;

    if (g_theLinkChecker != 6) {
        captainslog_fatal("Wrong operator new/delete linked in! Fix this...");
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
        captainslog_trace("Memory Manager initialising prior to main");

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
        if (g_dynamicMemoryAllocator != nullptr) {
            captainslog_dbgassert(g_memoryPoolFactory != nullptr, "hmm, no factory");

            if (g_memoryPoolFactory != nullptr) {
                g_memoryPoolFactory->Destroy_Dynamic_Memory_Allocator(g_dynamicMemoryAllocator);
                g_dynamicMemoryAllocator = nullptr;
            }

            delete g_memoryPoolFactory;
            g_memoryPoolFactory = nullptr;
        }
    }

    g_theMainInitFlag = false;
}

void *Create_W3D_Mem_Pool(char const *name, int size)
{
    ++g_theLinkChecker;
    Init_Memory_Manager_Pre_Main();
    MemoryPool *pool = g_memoryPoolFactory->Create_Memory_Pool(name, size, 0, 0);
    captainslog_dbgassert(pool != nullptr && pool->Get_Alloc_Size() == size, "bad w3d pool");
    return pool;
}

void *Allocate_From_W3D_Mem_Pool(void *pool, int size)
{
    MemoryPool *mempool = static_cast<MemoryPool *>(pool);
    captainslog_dbgassert(mempool != nullptr, "pool is null");
    captainslog_dbgassert(
        mempool != nullptr && mempool->Get_Alloc_Size() == size, "bad w3d pool size %s", mempool->Get_Pool_Name());
    return mempool->Allocate_Block();
}

void Free_From_W3D_Mem_Pool(void *pool, void *data)
{
    MemoryPool *mempool = static_cast<MemoryPool *>(pool);
    captainslog_dbgassert(mempool != nullptr, "pool is null");
    mempool->Free_Block(data);
}

// These all override the global news and deletes just by being linked.
void *operator new(size_t bytes)
{
    ++g_theLinkChecker;
    Init_Memory_Manager_Pre_Main();
    captainslog_dbgassert(g_dynamicMemoryAllocator, "must init memory manager before calling global operator new");

    return g_dynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void *operator new[](size_t bytes)
{
    ++g_theLinkChecker;
    Init_Memory_Manager_Pre_Main();
    captainslog_dbgassert(g_dynamicMemoryAllocator, "must init memory manager before calling global operator new");

    return g_dynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void operator delete(void *ptr) noexcept
{
    ++g_theLinkChecker;
    Init_Memory_Manager_Pre_Main();
    captainslog_dbgassert(g_dynamicMemoryAllocator, "must init memory manager before calling global operator delete");

    g_dynamicMemoryAllocator->Free_Bytes(ptr);
}

void operator delete[](void *ptr) noexcept
{
    ++g_theLinkChecker;
    Init_Memory_Manager_Pre_Main();
    captainslog_dbgassert(g_dynamicMemoryAllocator, "must init memory manager before calling global operator delete");

    g_dynamicMemoryAllocator->Free_Bytes(ptr);
}

#if __cplusplus >= 201402L
void operator delete(void *ptr, size_t sz) noexcept
{
    operator delete(ptr);
}

void operator delete[](void *ptr, size_t sz) noexcept
{
    operator delete[](ptr);
}
#endif

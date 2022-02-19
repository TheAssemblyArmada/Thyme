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

struct PoolInitRec;
class MemoryPool;
class MemoryPoolFactory;
class MemoryPoolSingleBlock;
class SimpleCriticalSectionClass;

class DynamicMemoryAllocator
{
    friend class MemoryPoolFactory;

public:
    DynamicMemoryAllocator();
    void Init(MemoryPoolFactory *factory, int32_t subpools, PoolInitRec const *const params);
    ~DynamicMemoryAllocator();
    MemoryPool *Find_Pool_For_Size(int32_t size);
    void Add_To_List(DynamicMemoryAllocator **head);
    void Remove_From_List(DynamicMemoryAllocator **head);
    void *Allocate_Bytes_No_Zero(int32_t bytes);
    void *Allocate_Bytes(int32_t bytes);
    void Free_Bytes(void *block);
    int32_t Get_Actual_Allocation_Size(int32_t bytes);
    void Reset();

    void *operator new(size_t size) { return Raw_Allocate_No_Zero(size); }
    void operator delete(void *obj) { Raw_Free(obj); }

private:
    MemoryPoolFactory *m_factory;
    DynamicMemoryAllocator *m_nextDmaInFactory;
    int32_t m_poolCount;
    int32_t m_usedBlocksInDma;
    MemoryPool *m_pools[8];
    MemoryPoolSingleBlock *m_rawBlocks;
};

#ifdef GAME_DLL
extern SimpleCriticalSectionClass *&g_dmaCriticalSection;
extern DynamicMemoryAllocator *&g_dynamicMemoryAllocator;
#else
extern SimpleCriticalSectionClass *g_dmaCriticalSection;
extern DynamicMemoryAllocator *g_dynamicMemoryAllocator;
#endif
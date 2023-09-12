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

class MemoryPoolFactory;
class MemoryPoolBlob;
class SimpleCriticalSectionClass;

#ifdef GAME_DLL
extern SimpleCriticalSectionClass *&g_memoryPoolCriticalSection;
#else
extern SimpleCriticalSectionClass *g_memoryPoolCriticalSection;
#endif

class MemoryPool
{
    friend class MemoryPoolBlob;
    friend class MemoryPoolFactory;
    friend class DynamicMemoryAllocator;

public:
    MemoryPool();
    ~MemoryPool();
    void Init(MemoryPoolFactory *factory, const char *name, int size, int count, int overflow);
    MemoryPoolBlob *Create_Blob(int count);
    int Free_Blob(MemoryPoolBlob *blob);
    void *Allocate_Block_No_Zero();
    void *Allocate_Block();
    void Free_Block(void *block);
    int Count_Blobs();
    int Release_Empties();
    void Reset();
    void Add_To_List(MemoryPool **head);
    void Remove_From_List(MemoryPool **head);
    int Get_Alloc_Size() { return m_allocationSize; }
    const char *Get_Pool_Name() { return m_poolName; }

    void *operator new(size_t size) throw() { return Raw_Allocate(size); }
    void operator delete(void *obj) { Raw_Free(obj); }

private:
    MemoryPoolFactory *m_factory;
    MemoryPool *m_nextPoolInFactory;
    const char *m_poolName;
    int m_allocationSize;
    int m_initialAllocationCount;
    int m_overflowAllocationCount;
    int m_usedBlocksInPool;
    int m_totalBlocksInPool;
    int m_peakUsedBlocksInPool;
    MemoryPoolBlob *m_firstBlob;
    MemoryPoolBlob *m_lastBlob;
    MemoryPoolBlob *m_firstBlobWithFreeBlocks;
};

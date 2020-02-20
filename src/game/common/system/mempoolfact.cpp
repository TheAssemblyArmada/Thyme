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
#include "mempoolfact.h"
#include "gamememoryinit.h"
#include "memdynalloc.h"
#include "mempool.h"
#include <captainslog.h>
#include <cstring>

using std::strcmp;

#ifndef GAME_DLL
MemoryPoolFactory *g_memoryPoolFactory = nullptr;
#endif

MemoryPoolFactory::~MemoryPoolFactory()
{
    for (MemoryPool *mp = m_firstPoolInFactory; m_firstPoolInFactory != nullptr; mp = m_firstPoolInFactory) {
        Destroy_Memory_Pool(mp);
    }

    for (DynamicMemoryAllocator *dma = m_firstDmaInFactory; m_firstDmaInFactory != nullptr; dma = m_firstDmaInFactory) {
        Destroy_Dynamic_Memory_Allocator(dma);
    }
}

MemoryPool *MemoryPoolFactory::Create_Memory_Pool(PoolInitRec const *params)
{
    return Create_Memory_Pool(
        params->pool_name, params->allocation_size, params->initial_allocation_count, params->overflow_allocation_count);
}

MemoryPool *MemoryPoolFactory::Create_Memory_Pool(const char *name, int size, int count, int overflow)
{
    MemoryPool *pool = Find_Memory_Pool(name);

    if (pool != nullptr) {
        captainslog_dbgassert(pool->m_allocationSize == size, "Pool size mismatch");

        return pool;
    }

    User_Memory_Adjust_Pool_Size(name, count, overflow);

    // Count and overflow should never end up as 0 from adjustment.
    captainslog_relassert(count > 0 && overflow > 0, 0xDEAD0002, "Count and overflow are 0 for pool '%s'.", name);

    pool = new MemoryPool;
    pool->Init(this, name, size, count, overflow);
    pool->Add_To_List(&m_firstPoolInFactory);

    return pool;
}

void MemoryPoolFactory::Destroy_Memory_Pool(MemoryPool *pool)
{
    // Can't destroy a none existent pool.
    if (pool == nullptr) {
        return;
    }

    captainslog_dbgassert(pool->m_usedBlocksInPool == 0, "Destroying none empty pool.");

    pool->Remove_From_List(&m_firstPoolInFactory);
    delete pool;
}

MemoryPool *MemoryPoolFactory::Find_Memory_Pool(const char *name)
{
    MemoryPool *pool = nullptr;

    // Go through the pools and break on matching requested name.
    for (pool = m_firstPoolInFactory; pool != nullptr; pool = pool->m_nextPoolInFactory) {
        if (strcmp(pool->m_poolName, name) == 0) {
            break;
        }
    }

    return pool;
}

DynamicMemoryAllocator *MemoryPoolFactory::Create_Dynamic_Memory_Allocator(int subpools, PoolInitRec const *const params)
{
    DynamicMemoryAllocator *allocator = new DynamicMemoryAllocator;
    allocator->Init(this, subpools, params);
    allocator->Add_To_List(&m_firstDmaInFactory);

    return allocator;
}

void MemoryPoolFactory::Destroy_Dynamic_Memory_Allocator(DynamicMemoryAllocator *allocator)
{
    if (allocator == nullptr) {
        return;
    }

    allocator->Remove_From_List(&m_firstDmaInFactory);
    delete allocator;
}

void MemoryPoolFactory::Reset()
{
    for (MemoryPool *mp = m_firstPoolInFactory; mp != nullptr; mp = mp->m_nextPoolInFactory) {
        mp->Reset();
    }

    for (DynamicMemoryAllocator *dma = m_firstDmaInFactory; dma != nullptr; dma = dma->m_nextDmaInFactory) {
        dma->Reset();
    }
}

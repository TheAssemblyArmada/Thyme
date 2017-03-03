////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MEMPOOLFACT.CPP
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
#include "mempoolfact.h"
#include "gamedebug.h"
#include "gamememoryinit.h"
#include "memdynalloc.h"
#include "mempool.h"

////////////////////
// MemoryPoolFactory
////////////////////

MemoryPoolFactory::~MemoryPoolFactory()
{
    for ( MemoryPool *mp = FirstPoolInFactory; FirstPoolInFactory != nullptr; mp = FirstPoolInFactory ) {
        Destroy_Memory_Pool(mp);
    }

    for ( DynamicMemoryAllocator *dma = FirstDmaInFactory; FirstDmaInFactory != nullptr; dma = FirstDmaInFactory ) {
        Destroy_Dynamic_Memory_Allocator(dma);
    }
}

MemoryPool *MemoryPoolFactory::Create_Memory_Pool(PoolInitRec const *params)
{
    return Create_Memory_Pool(
        params->PoolName,
        params->AllocationSize,
        params->InitialAllocationCount,
        params->OverflowAllocationCount
    );
}

MemoryPool *MemoryPoolFactory::Create_Memory_Pool(char const *name, int size, int count, int overflow)
{
    MemoryPool *pool = Find_Memory_Pool(name);

    if ( pool != nullptr ) {
        ASSERT_PRINT(pool->AllocationSize == size, "Pool size mismatch");

        return pool;
    }

    User_Memory_Adjust_Pool_Size(name, count, overflow);

    //
    // Count and overflow should never end up as 0 from adjustment.
    //
    ASSERT_THROW(count > 0 && overflow > 0, 0xDEAD0002);

    pool = new MemoryPool;
    pool->Init(this, name, size, count, overflow);
    pool->Add_To_List(&FirstPoolInFactory);

    return pool;
}

void MemoryPoolFactory::Destroy_Memory_Pool(MemoryPool *pool)
{
    //
    // Can't destroy a none existent pool.
    //
    if ( pool == nullptr ) {
        return;
    }

    ASSERT_PRINT(pool->UsedBlocksInPool == 0, "Destroying none empty pool.");

    pool->Remove_From_List(&FirstPoolInFactory);
    delete pool;
}

MemoryPool *MemoryPoolFactory::Find_Memory_Pool(char const *name)
{
    MemoryPool *pool = nullptr;

    //
    // Go through the pools and break on matching requested name.
    //
    for ( pool = FirstPoolInFactory; pool != nullptr; pool = pool->NextPoolInFactory ) {
        if ( strcmp(pool->PoolName, name) == 0 ) {
            break;
        }
    }

    return pool;
}

DynamicMemoryAllocator *MemoryPoolFactory::Create_Dynamic_Memory_Allocator(int subpools, PoolInitRec const *const params)
{
    DynamicMemoryAllocator *allocator = new DynamicMemoryAllocator;
    allocator->Init(this, subpools, params);
    allocator->Add_To_List(&FirstDmaInFactory);

    return allocator;
}

void MemoryPoolFactory::Destroy_Dynamic_Memory_Allocator(DynamicMemoryAllocator *allocator)
{
    if ( allocator == nullptr ) {
        return;
    }

    allocator->Remove_From_List(&FirstDmaInFactory);
    delete allocator;
}

void MemoryPoolFactory::Reset()
{
    for ( MemoryPool *mp = FirstPoolInFactory; mp != nullptr; mp = mp->NextPoolInFactory ) {
        mp->Reset();
    }

    for ( DynamicMemoryAllocator *dma = FirstDmaInFactory; dma != nullptr; dma = dma->NextDmaInFactory ) {
        dma->Reset();
    }
}


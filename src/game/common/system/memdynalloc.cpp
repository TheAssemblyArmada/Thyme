////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MEMDYNALLOC.CPP
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
#include "memdynalloc.h"
#include "critsection.h"
#include "gamememoryinit.h"
#include "memblob.h"
#include "memblock.h"
#include "mempool.h"
#include "mempoolfact.h"

SimpleCriticalSectionClass *DmaCriticalSection = nullptr;

DynamicMemoryAllocator::DynamicMemoryAllocator() :
    Factory(nullptr),
    NextDmaInFactory(nullptr),
    PoolCount(0),
    UsedBlocksInDma(0),
    RawBlocks(0)
{
    memset(Pools, 0, sizeof(Pools));
}

void DynamicMemoryAllocator::Init(MemoryPoolFactory *factory, int subpools, PoolInitRec const *const params)
{
    PoolInitRec const defaults[7] = {
        { "dmaPool_16",   16,   64, 64 },
        { "dmaPool_32",   32,   64, 64 },
        { "dmaPool_64",   64,   64, 64 },
        { "dmaPool_128",  128,  64, 64 },
        { "dmaPool_256",  256,  64, 64 },
        { "dmaPool_512",  512,  64, 64 },
        { "dmaPool_1024", 1024, 64, 64 },
    };

    PoolInitRec const *init_list = params;
    PoolCount = subpools;

    //
    // If we didn't get passed any initialisation or no count of sub pools, use the defaults
    //
    if ( PoolCount <= 0 || init_list == nullptr ) {
        PoolCount = 7;
        init_list = defaults;
    }

    Factory = factory;
    UsedBlocksInDma = 0;

    if ( PoolCount > 8 ) {
        PoolCount = 8;
    }

    for ( int i = 0; i < PoolCount; ++i ) {
        Pools[i] = Factory->Create_Memory_Pool(&init_list[i]);
    }
}

DynamicMemoryAllocator::~DynamicMemoryAllocator()
{
    ASSERT_PRINT(UsedBlocksInDma, "Destroying none empty DMA.");

    for ( int i = 0; i < PoolCount; ++i ) {
        Factory->Destroy_Memory_Pool(Pools[i]);
        Pools[i] = nullptr;
    }

    for ( MemoryPoolSingleBlock *b = RawBlocks; b != nullptr; b = RawBlocks ) {
        Free_Bytes(b->Get_User_Data());
    }
}

MemoryPool *DynamicMemoryAllocator::Find_Pool_For_Size(int size)
{
    if ( PoolCount <= 0 ) {
        return nullptr;
    }

    for ( int i = 0; i < PoolCount; ++i ) {
        if ( size <= Pools[i]->AllocationSize ) {
            return Pools[i];
        }
    }

    return nullptr;
}

void DynamicMemoryAllocator::Add_To_List(DynamicMemoryAllocator **head)
{
    NextDmaInFactory = *head;
    *head = this;
}

void DynamicMemoryAllocator::Remove_From_List(DynamicMemoryAllocator **head)
{
    DynamicMemoryAllocator *prev_dma = nullptr;
    DynamicMemoryAllocator *dma = *head;

    if ( *head == nullptr ) {
        return;
    }

    while ( dma != this ) {
        prev_dma = dma;
        dma = dma->NextDmaInFactory;

        if ( dma == nullptr ) {
            return;
        }
    }

    if ( prev_dma != nullptr ) {
        prev_dma->NextDmaInFactory = NextDmaInFactory;
    } else {
        *head = NextDmaInFactory;
    }
}

void *DynamicMemoryAllocator::Allocate_Bytes_No_Zero(int bytes)
{
    ScopedCriticalSectionClass cs(DmaCriticalSection);

    MemoryPool *mp = Find_Pool_For_Size(bytes);
    void *block;

    if ( mp != nullptr ) {
        block = mp->Allocate_Block_No_Zero();
    } else {
        block = MemoryPoolSingleBlock::Raw_Allocate_Single_Block(&RawBlocks, bytes)->Get_User_Data();
    }

    ++UsedBlocksInDma;

    return block;
}

void *DynamicMemoryAllocator::Allocate_Bytes(int bytes)
{
    void *block = Allocate_Bytes_No_Zero(bytes);
    memset(block, 0, bytes);

    return block;
}

void DynamicMemoryAllocator::Free_Bytes(void *block)
{
    if ( block == nullptr ) {
        return;
    }

    ScopedCriticalSectionClass cs(DmaCriticalSection);

    MemoryPoolSingleBlock *sblock = MemoryPoolSingleBlock::Recover_Block_From_User_Data(block);

    if ( sblock->OwningBlob != nullptr ) {
        sblock->OwningBlob->OwningPool->Free_Block(block);
    } else {
        sblock->Remove_Block_From_List(&RawBlocks);
        Raw_Free(sblock);
    }

    --UsedBlocksInDma;
}

int DynamicMemoryAllocator::Get_Actual_Allocation_Size(int bytes)
{
    MemoryPool *mp = Find_Pool_For_Size(bytes);

    if ( mp != nullptr ) {
        return mp->AllocationSize;
    }

    return bytes;
}

void DynamicMemoryAllocator::Reset()
{
    for ( int i = 0; i < PoolCount; ++i ) {
        if ( Pools[i] != nullptr ) {
            Pools[i]->Reset();
        }
    }

    for ( MemoryPoolSingleBlock *sb = RawBlocks; sb != nullptr; sb = RawBlocks ) {
        Free_Bytes(sb->Get_User_Data());
    }

    UsedBlocksInDma = 0;
}


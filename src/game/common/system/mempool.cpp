////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MEMPOOL.H
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
#include "mempool.h"
#include "critsection.h"
#include "memblob.h"
#include "memblock.h"
#include "minmax.h"

SimpleCriticalSectionClass *MemoryPoolCriticalSection = nullptr;

/////////////
// MemoryPool
/////////////

MemoryPool::MemoryPool() :
    Factory(nullptr),
    NextPoolInFactory(nullptr),
    PoolName(""),
    AllocationSize(0),
    InitialAllocationCount(0),
    OverflowAllocationCount(0),
    UsedBlocksInPool(0),
    TotalBlocksInPool(0),
    PeakUsedBlocksInPool(0),
    FirstBlob(nullptr),
    LastBlob(nullptr),
    FirstBlobWithFreeBlocks(nullptr)
{

}

MemoryPool::~MemoryPool()
{
    for ( MemoryPoolBlob *b = FirstBlob; b != nullptr; b = FirstBlob ) {
        Free_Blob(b);
    }
}

void MemoryPool::Init(MemoryPoolFactory *factory, char const *name, int size, int count, int overflow)
{
    Factory = factory;
    PoolName = name;
    AllocationSize = Round_Up_Word_Size(size);
    OverflowAllocationCount = overflow;
    InitialAllocationCount = count;
    UsedBlocksInPool = 0;
    TotalBlocksInPool = 0;
    PeakUsedBlocksInPool = 0;
    FirstBlob = nullptr;
    LastBlob = nullptr;
    FirstBlobWithFreeBlocks = nullptr;
    Create_Blob(count);
}

MemoryPoolBlob *MemoryPool::Create_Blob(int count)
{
    MemoryPoolBlob *blob = new MemoryPoolBlob;
    blob->Init_Blob(this, count);
    blob->Add_Blob_To_List(&FirstBlob, &LastBlob);

    ASSERT_PRINT(FirstBlobWithFreeBlocks == nullptr, "Expected nullptr here");

    FirstBlobWithFreeBlocks = blob;
    TotalBlocksInPool += count;

    return blob;
}

int MemoryPool::Free_Blob(MemoryPoolBlob *blob)
{
    ASSERT_PRINT(blob->OwningPool == this, "Blob does not belong to this pool");

    blob->Remove_Blob_From_List(&FirstBlob, &LastBlob);

    if ( FirstBlobWithFreeBlocks == blob ) {
        FirstBlobWithFreeBlocks = FirstBlob;
    }

    int blob_alloc = blob->TotalBlocksInBlob * AllocationSize + sizeof(*blob);
    UsedBlocksInPool -= blob->UsedBlocksInBlob;
    TotalBlocksInPool -= blob->TotalBlocksInBlob;

    delete blob;

    return blob_alloc;
}

void *MemoryPool::Allocate_Block_No_Zero()
{
    ScopedCriticalSectionClass scs(MemoryPoolCriticalSection);

    if ( FirstBlobWithFreeBlocks != nullptr && FirstBlobWithFreeBlocks->FirstFreeBlock == nullptr ) {
        MemoryPoolBlob *i;
        for ( i = FirstBlob; i != nullptr; i = i->NextBlob ) {
            if ( i->FirstFreeBlock != nullptr ) {
                break;
            }
        }

        FirstBlobWithFreeBlocks = i;
    }

    if ( FirstBlobWithFreeBlocks == nullptr ) {
        ASSERT_THROW(OverflowAllocationCount != 0, 0xDEAD0002);
        Create_Blob(OverflowAllocationCount);
    }

    MemoryPoolSingleBlock *block = FirstBlobWithFreeBlocks->Allocate_Single_Block();
    ++UsedBlocksInPool;

    //TODO convert to MAX()
    //if ( PeakUsedBlocksInPool < UsedBlocksInPool ) {
    //    PeakUsedBlocksInPool = UsedBlocksInPool;
    //}

    PeakUsedBlocksInPool = MAX(PeakUsedBlocksInPool, UsedBlocksInPool);

    return block->Get_User_Data();
}

void *MemoryPool::Allocate_Block()
{
    void *block = Allocate_Block_No_Zero();
    memset(block, 0, AllocationSize);

    return block;
}

void MemoryPool::Free_Block(void *block)
{
    if ( block == nullptr ) {
        return;
    }

    ScopedCriticalSectionClass scs(MemoryPoolCriticalSection);
    MemoryPoolSingleBlock *mp_block = MemoryPoolSingleBlock::Recover_Block_From_User_Data(block);
    MemoryPoolBlob *mp_blob = mp_block->OwningBlob;

    ASSERT_PRINT(mp_blob != nullptr && mp_blob->OwningPool == this, "Block is not part of this pool");

    mp_blob->Free_Single_Block(mp_block);

    if ( FirstBlobWithFreeBlocks == nullptr ) {
        FirstBlobWithFreeBlocks = mp_blob;
        --UsedBlocksInPool;
    }
}

int MemoryPool::Count_Blobs()
{
    int count = 0;

    for ( MemoryPoolBlob *i = FirstBlob; i != nullptr; i = i->NextBlob ) {
        ++count;
    }

    return count;
}

int MemoryPool::Release_Empties()
{
    int count = 0;

    for ( MemoryPoolBlob *i = FirstBlob; i != nullptr; i = i->NextBlob ) {
        if ( i->UsedBlocksInBlob == 0 ) {
            count += Free_Blob(i);
        }
    }

    return  count;
}

void MemoryPool::Reset()
{
    for ( MemoryPoolBlob *i = FirstBlob; i != nullptr; i = FirstBlob ) {
        Free_Blob(i);
    }

    FirstBlob = nullptr;
    LastBlob = nullptr;
    FirstBlob = nullptr;

    Init(Factory, PoolName, AllocationSize, InitialAllocationCount, OverflowAllocationCount);
}

void MemoryPool::Add_To_List(MemoryPool **head)
{
    NextPoolInFactory = *head;
    *head = this;
}

void MemoryPool::Remove_From_List(MemoryPool **head)
{
    if ( *head == nullptr ) {
        return;
    }

    MemoryPool *check = *head;
    MemoryPool *last = nullptr;

    while ( check != this ) {
        last = check;
        check = check->NextPoolInFactory;

        if ( check == nullptr ) {
            return;
        }
    }

    if ( last != nullptr ) {
        last->NextPoolInFactory = NextPoolInFactory;
    } else {
        *head = NextPoolInFactory;
    }
}


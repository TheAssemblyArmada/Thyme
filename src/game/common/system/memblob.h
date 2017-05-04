////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MEMBLOB.H
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

#ifndef MEMBLOB_H
#define MEMBLOB_H

#include "rawalloc.h"
#include "memblock.h"

class MemoryPool;

class MemoryPoolBlob
{
public:
    MemoryPoolBlob();
    ~MemoryPoolBlob();
    void Init_Blob(MemoryPool *owning_pool, int count);
    void Add_Blob_To_List(MemoryPoolBlob **head, MemoryPoolBlob **tail);
    void Remove_Blob_From_List(MemoryPoolBlob **head, MemoryPoolBlob **tail);
    MemoryPoolSingleBlock *Allocate_Single_Block();
    void Free_Single_Block(MemoryPoolSingleBlock *block);

    void *operator new(size_t size) throw()
    {
        return Raw_Allocate(size);
    }

    void operator delete(void *obj)
    {
        Raw_Free(obj);
    }

    friend class MemoryPool;
    friend class DynamicMemoryAllocator;
private:
    MemoryPool *OwningPool;
    MemoryPoolBlob *NextBlob;
    MemoryPoolBlob *PrevBlob;
    MemoryPoolSingleBlock *FirstFreeBlock;
    int UsedBlocksInBlob;
    int TotalBlocksInBlob;
    char *BlockData;
};

inline MemoryPoolBlob::MemoryPoolBlob() :
    OwningPool(nullptr),
    NextBlob(nullptr),
    PrevBlob(nullptr),
    FirstFreeBlock(nullptr),
    UsedBlocksInBlob(0),
    TotalBlocksInBlob(0),
    BlockData(nullptr)
{

}

inline MemoryPoolBlob::~MemoryPoolBlob()
{
    Raw_Free(BlockData);
}

inline void MemoryPoolBlob::Add_Blob_To_List(MemoryPoolBlob **head, MemoryPoolBlob **tail)
{
    NextBlob = 0;
    PrevBlob = *tail;

    if ( *tail != nullptr ) {
        (*tail)->NextBlob = this;
    }

    if ( *head == nullptr ) {
        *head = this;
    }

    *tail = this;
}

inline void MemoryPoolBlob::Remove_Blob_From_List(MemoryPoolBlob **head, MemoryPoolBlob **tail)
{
    if ( *head == this ) {
        *head = NextBlob;
    } else {
        PrevBlob->NextBlob = NextBlob;
    }

    if ( *tail == this ) {
        *tail = PrevBlob;
    } else {
        NextBlob->PrevBlob = PrevBlob;
    }
}

inline MemoryPoolSingleBlock *MemoryPoolBlob::Allocate_Single_Block()
{
    //ASSERT_PRINT(UsedBlocksInBlob < TotalBlocksInBlob, "Trying to allocate when all blocks allocated in blob for pool %s\n", OwningPool->PoolName);
    //ASSERT_PRINT(FirstFreeBlock != nullptr, "Trying to allocated block from blob with null FirstFreeBlock for pool %s\n", OwningPool->PoolName);
    MemoryPoolSingleBlock *block = FirstFreeBlock;
    FirstFreeBlock = block->NextBlock;
    ++UsedBlocksInBlob;

    return block;
}

inline void MemoryPoolBlob::Free_Single_Block(MemoryPoolSingleBlock *block)
{
    block->Add_Block_To_List(&FirstFreeBlock);
    --UsedBlocksInBlob;
}

#endif

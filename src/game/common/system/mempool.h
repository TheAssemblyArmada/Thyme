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
#pragma once

#ifndef MEMPOOL_H
#define MEMPOOL_H

#include "rawalloc.h"

class MemoryPoolFactory;
class MemoryPoolBlob;
class SimpleCriticalSectionClass;

// Allocated a critical section in WinMain, hooked to original currently.
extern SimpleCriticalSectionClass* MemoryPoolCriticalSection;

class MemoryPool
{
public:
    MemoryPool();
    ~MemoryPool();
    void Init(MemoryPoolFactory *factory, char const *name, int size, int count, int overflow);
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

    int Get_Alloc_Size() { return AllocationSize; }

    void *operator new(size_t size) throw()
    {
        return Raw_Allocate(size);
    }

    void operator delete(void *obj)
    {
        Raw_Free(obj);
    }

    friend class MemoryPoolBlob;
    friend class MemoryPoolFactory;
    friend class DynamicMemoryAllocator;

private:
    MemoryPoolFactory *Factory;
    MemoryPool *NextPoolInFactory;
    char const *PoolName;
    int AllocationSize;
    int InitialAllocationCount;
    int OverflowAllocationCount;
    int UsedBlocksInPool;
    int TotalBlocksInPool;
    int PeakUsedBlocksInPool;
    MemoryPoolBlob *FirstBlob;
    MemoryPoolBlob *LastBlob;
    MemoryPoolBlob *FirstBlobWithFreeBlocks;
};

#endif

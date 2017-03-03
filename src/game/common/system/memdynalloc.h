////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MEMDYNALLOC.H
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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _MEMDYNALLOC_H_
#define _MEMDYNALLOC_H_

#include "rawalloc.h"
#include "hooker.h"

struct PoolInitRec;
class MemoryPool;
class MemoryPoolFactory;
class MemoryPoolSingleBlock;
class SimpleCriticalSectionClass;

extern SimpleCriticalSectionClass* DmaCriticalSection;

#define TheDynamicMemoryAllocator (Make_Global<DynamicMemoryAllocator*>(0x00A29B98))

class DynamicMemoryAllocator
{
public:
    DynamicMemoryAllocator();
    void Init(MemoryPoolFactory *factory, int subpools, PoolInitRec const *const params);
    ~DynamicMemoryAllocator();
    MemoryPool *Find_Pool_For_Size(int size);
    void Add_To_List(DynamicMemoryAllocator **head);
    void Remove_From_List(DynamicMemoryAllocator **head);
    void *Allocate_Bytes_No_Zero(int bytes);
    void *Allocate_Bytes(int bytes);
    void Free_Bytes(void *block);
    int Get_Actual_Allocation_Size(int bytes);
    void Reset();

    void *operator new(size_t size)
    {
        return Raw_Allocate_No_Zero(size);
    }

    void operator delete(void *obj)
    {
        Raw_Free(obj);
    }

    friend class MemoryPoolFactory;

private:
    MemoryPoolFactory *Factory;
    DynamicMemoryAllocator *NextDmaInFactory;
    int PoolCount;
    int UsedBlocksInDma;
    MemoryPool *Pools[8];
    MemoryPoolSingleBlock *RawBlocks;
};



#endif

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
    MemoryPool *m_owningPool;
    MemoryPoolBlob *m_nextBlob;
    MemoryPoolBlob *m_prevBlob;
    MemoryPoolSingleBlock *m_firstFreeBlock;
    int m_usedBlocksInBlob;
    int m_totalBlocksInBlob;
    char *m_blockData;
};

inline MemoryPoolBlob::MemoryPoolBlob() :
    m_owningPool(nullptr),
    m_nextBlob(nullptr),
    m_prevBlob(nullptr),
    m_firstFreeBlock(nullptr),
    m_usedBlocksInBlob(0),
    m_totalBlocksInBlob(0),
    m_blockData(nullptr)
{

}

inline MemoryPoolBlob::~MemoryPoolBlob()
{
    Raw_Free(m_blockData);
}

inline void MemoryPoolBlob::Add_Blob_To_List(MemoryPoolBlob **head, MemoryPoolBlob **tail)
{
    m_nextBlob = 0;
    m_prevBlob = *tail;

    if ( *tail != nullptr ) {
        (*tail)->m_nextBlob = this;
    }

    if ( *head == nullptr ) {
        *head = this;
    }

    *tail = this;
}

inline void MemoryPoolBlob::Remove_Blob_From_List(MemoryPoolBlob **head, MemoryPoolBlob **tail)
{
    if ( *head == this ) {
        *head = m_nextBlob;
    } else {
        m_prevBlob->m_nextBlob = m_nextBlob;
    }

    if ( *tail == this ) {
        *tail = m_prevBlob;
    } else {
        m_nextBlob->m_prevBlob = m_prevBlob;
    }
}

inline MemoryPoolSingleBlock *MemoryPoolBlob::Allocate_Single_Block()
{
    //ASSERT_PRINT(m_usedBlocksInBlob < m_totalBlocksInBlob, "Trying to allocate when all blocks allocated in blob for pool %s\n", m_owningPool->m_poolName);
    //ASSERT_PRINT(m_firstFreeBlock != nullptr, "Trying to allocated block from blob with null m_firstFreeBlock for pool %s\n", m_owningPool->m_poolName);
    MemoryPoolSingleBlock *block = m_firstFreeBlock;
    m_firstFreeBlock = block->m_nextBlock;
    ++m_usedBlocksInBlob;

    return block;
}

inline void MemoryPoolBlob::Free_Single_Block(MemoryPoolSingleBlock *block)
{
    block->Add_Block_To_List(&m_firstFreeBlock);
    --m_usedBlocksInBlob;
}

#endif

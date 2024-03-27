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
#include "memblock.h"
#include "rawalloc.h"

class MemoryPool;

class MemoryPoolBlob
{
    friend class MemoryPool;
    friend class DynamicMemoryAllocator;

public:
    MemoryPoolBlob();
    ~MemoryPoolBlob();
    void Init_Blob(MemoryPool *owning_pool, int count);
    void Add_Blob_To_List(MemoryPoolBlob **head, MemoryPoolBlob **tail);
    void Remove_Blob_From_List(MemoryPoolBlob **head, MemoryPoolBlob **tail);
    MemoryPoolSingleBlock *Allocate_Single_Block();
    void Free_Single_Block(MemoryPoolSingleBlock *block);

    void *operator new(size_t size) throw() { return Raw_Allocate(size); }
    void operator delete(void *obj) { Raw_Free(obj); }

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

    if (*tail != nullptr) {
        (*tail)->m_nextBlob = this;
    }

    if (*head == nullptr) {
        *head = this;
    }

    *tail = this;
}

inline void MemoryPoolBlob::Remove_Blob_From_List(MemoryPoolBlob **head, MemoryPoolBlob **tail)
{
    if (*head == this) {
        *head = m_nextBlob;
    } else {
        m_prevBlob->m_nextBlob = m_nextBlob;
    }

    if (*tail == this) {
        *tail = m_prevBlob;
    } else {
        m_nextBlob->m_prevBlob = m_prevBlob;
    }
}

inline MemoryPoolSingleBlock *MemoryPoolBlob::Allocate_Single_Block()
{
    captainslog_dbgassert(m_firstFreeBlock != nullptr, "Trying to allocated block from blob with no free blocks.");
    MemoryPoolSingleBlock *block = m_firstFreeBlock;
    m_firstFreeBlock = block->Get_Next_Free();
    ++m_usedBlocksInBlob;

    return block;
}

inline void MemoryPoolBlob::Free_Single_Block(MemoryPoolSingleBlock *block)
{
    captainslog_relassert(
        block->m_owningBlob == this, 0xDEAD0002, "Attempting to free a block that does not belong to this blob.");
    block->Set_Next_Free(m_firstFreeBlock);
    --m_usedBlocksInBlob;
    m_firstFreeBlock = block;
}
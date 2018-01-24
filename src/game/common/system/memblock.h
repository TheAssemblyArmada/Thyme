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

#ifndef MEMBLOCK_H
#define MEMBLOCK_H

#include "always.h"
#include "gamedebug.h"
#include "rawalloc.h"

class MemoryPoolBlob;
class MemoryPoolFactory;

class MemoryPoolSingleBlock
{
    friend class MemoryPoolBlob;
    friend class MemoryPool;
    friend class DynamicMemoryAllocator;

public:
    MemoryPoolSingleBlock() : m_owningBlob(nullptr), m_nextBlock(nullptr), m_prevBlock(nullptr) {}
    void Init_Block(int size, MemoryPoolBlob *owning_blob, MemoryPoolFactory *owning_fact);
    void Remove_Block_From_List(MemoryPoolSingleBlock **list_head);
    void Add_Block_To_List(MemoryPoolSingleBlock *list_head);
    void Set_Next_Free(MemoryPoolSingleBlock *next);
    MemoryPoolSingleBlock *Get_Next_Free();
    void *Get_User_Data() { return reinterpret_cast<void *>(&this[1]); }
    static MemoryPoolSingleBlock *Recover_Block_From_User_Data(void *data);
    static MemoryPoolSingleBlock *Raw_Allocate_Single_Block(MemoryPoolSingleBlock **list_head, int size, MemoryPoolFactory *owning_fact);

private:
    MemoryPoolBlob *m_owningBlob;
    MemoryPoolSingleBlock *m_nextBlock;
    MemoryPoolSingleBlock *m_prevBlock;
};

inline void MemoryPoolSingleBlock::Init_Block(int size, MemoryPoolBlob *owning_blob, MemoryPoolFactory *owning_fact)
{
    ASSERT_THROW_PRINT(owning_fact != nullptr, 0xDEAD0002, "Owning factory is nullptr.\n");
    m_nextBlock = 0;
    m_prevBlock = 0;
    m_owningBlob = owning_blob;
}

inline void MemoryPoolSingleBlock::Remove_Block_From_List(MemoryPoolSingleBlock **list_head)
{
    ASSERT_PRINT(m_owningBlob == nullptr, "This function should only be used on raw blocks.\n");

    // Do we have previous? If not, we are the head?
    if ( m_prevBlock != nullptr ) {
        ASSERT_PRINT(this != *list_head, "Bad list linkage");
        m_prevBlock->m_nextBlock = m_nextBlock;
    } else {
        *list_head = m_nextBlock;
    }

    if ( m_nextBlock != nullptr ) {
        m_nextBlock->m_prevBlock = m_prevBlock;
    }
}

inline void MemoryPoolSingleBlock::Add_Block_To_List(MemoryPoolSingleBlock *list_head)
{
    m_nextBlock = list_head;

    if ( list_head != nullptr ) {
        list_head->m_prevBlock = this;
    }
}

inline void MemoryPoolSingleBlock::Set_Next_Free(MemoryPoolSingleBlock *next)
{
    ASSERT_THROW_PRINT(m_owningBlob != nullptr, 0xDEAD0002, "Must be called on a blob block.\n");
    Add_Block_To_List(next);
}

inline MemoryPoolSingleBlock * MemoryPoolSingleBlock::Get_Next_Free()
{
    ASSERT_THROW_PRINT(m_owningBlob != nullptr, 0xDEAD0002, "Must be called on a blob block.\n");

    return m_nextBlock;
}

inline MemoryPoolSingleBlock *MemoryPoolSingleBlock::Recover_Block_From_User_Data(void *data)
{
    if ( data != nullptr ) {
        return reinterpret_cast<MemoryPoolSingleBlock *>(static_cast<char *>(data) - sizeof(MemoryPoolSingleBlock));
    } else {
        ASSERT_PRINT(false, "null data");

        return nullptr;
    }
}

inline MemoryPoolSingleBlock *MemoryPoolSingleBlock::Raw_Allocate_Single_Block(MemoryPoolSingleBlock **list_head, int size, MemoryPoolFactory *owning_fact)
{
    MemoryPoolSingleBlock *block = static_cast<MemoryPoolSingleBlock*>(Raw_Allocate_No_Zero(Round_Up_Word_Size(size) + sizeof(MemoryPoolSingleBlock)));
    block->Init_Block(size, nullptr, owning_fact);
    block->Add_Block_To_List(*list_head);
    *list_head = block;

    return block;
}

#endif // MEMBLOCK_H

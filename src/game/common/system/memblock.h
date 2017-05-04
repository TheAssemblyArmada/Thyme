////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MEMBLOCK.H
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

#ifndef MEMBLOCK_H
#define MEMBLOCK_H

#include "gamedebug.h"
#include "rawalloc.h"

class MemoryPoolBlob;

class MemoryPoolSingleBlock
{
public:
    MemoryPoolSingleBlock() : OwningBlob(nullptr), NextBlock(nullptr), PrevBlock(nullptr) {}
    void Init_Block(int size, MemoryPoolBlob *owning_blob);
    void Remove_Block_From_List(MemoryPoolSingleBlock **list_head);
    void Add_Block_To_List(MemoryPoolSingleBlock **list_head);
    void *Get_User_Data() { return reinterpret_cast<void *>(&this[1]); }
    static MemoryPoolSingleBlock *Recover_Block_From_User_Data(void *data);
    static MemoryPoolSingleBlock *Raw_Allocate_Single_Block(MemoryPoolSingleBlock **list_head, int size);

    friend class MemoryPoolBlob;
    friend class MemoryPool;
    friend class DynamicMemoryAllocator;

private:
    MemoryPoolBlob *OwningBlob;
    MemoryPoolSingleBlock *NextBlock;
    MemoryPoolSingleBlock *PrevBlock;
};

inline void MemoryPoolSingleBlock::Init_Block(int size, MemoryPoolBlob *owning_blob)
{
    NextBlock = 0;
    PrevBlock = 0;
    OwningBlob = owning_blob;
}

inline void MemoryPoolSingleBlock::Remove_Block_From_List(MemoryPoolSingleBlock **list_head)
{
    ASSERT_PRINT(OwningBlob == nullptr, "This function should only be used on raw blocks.\n");

    // Do we have previous? If not, we are the head?
    if ( PrevBlock != nullptr ) {
        ASSERT_PRINT(this != *list_head, "Bad list linkage");
        PrevBlock->NextBlock = NextBlock;
    } else {
        *list_head = NextBlock;
    }

    if ( NextBlock != nullptr ) {
        NextBlock->PrevBlock = PrevBlock;
    }
}

inline void MemoryPoolSingleBlock::Add_Block_To_List(MemoryPoolSingleBlock **list_head)
{
    NextBlock = *list_head;

    if ( *list_head != nullptr ) {
        (*list_head)->PrevBlock = this;
    }

    //*list_head = this;
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

inline MemoryPoolSingleBlock *MemoryPoolSingleBlock::Raw_Allocate_Single_Block(MemoryPoolSingleBlock **list_head, int size)
{
    MemoryPoolSingleBlock *block = static_cast<MemoryPoolSingleBlock*>(Raw_Allocate_No_Zero(Round_Up_Word_Size(size) + sizeof(MemoryPoolSingleBlock)));
    block->Init_Block(size, nullptr);
    block->Add_Block_To_List(list_head);

    return block;
}

#endif // _MEMBLOCK_H

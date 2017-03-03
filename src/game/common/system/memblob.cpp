////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MEMBLOB.CPP
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
#include "memblob.h"
#include "memblock.h"
#include "mempool.h"

void MemoryPoolBlob::Init_Blob(MemoryPool *owning_pool, int count)
{
    ASSERT_PRINT(BlockData == nullptr, "Init called on blob with none null data for pool %s\n", owning_pool->PoolName);

    OwningPool = owning_pool;
    TotalBlocksInBlob = count;
    UsedBlocksInBlob = 0;

    int alloc_size = Round_Up_Word_Size(owning_pool->AllocationSize) + sizeof(MemoryPoolSingleBlock);
    BlockData = static_cast<char *>(Raw_Allocate(alloc_size * TotalBlocksInBlob));
    char *current_block = BlockData;

    for ( int i = TotalBlocksInBlob - 1; i >= 0; --i ) {
        MemoryPoolSingleBlock *block_header = reinterpret_cast<MemoryPoolSingleBlock *>(current_block);
        block_header->Init_Block(0, this);

        //
        // Move to next block and get poointer to next header. Mask ensures its null
        // on last iteration.
        //
        if ( i > 0 ) {
            current_block = current_block + alloc_size;
            MemoryPoolSingleBlock *next = reinterpret_cast<MemoryPoolSingleBlock *>(current_block);

            block_header->NextBlock = next;
            next->PrevBlock = block_header;
        } else {
            block_header->NextBlock = nullptr;
        }
    }

    FirstFreeBlock = reinterpret_cast<MemoryPoolSingleBlock *>(BlockData);
}

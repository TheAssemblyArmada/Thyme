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
#include "memblob.h"
#include "memblock.h"
#include "mempool.h"

void MemoryPoolBlob::Init_Blob(MemoryPool *owning_pool, int count)
{
    captainslog_dbgassert(
        m_blockData == nullptr, "Init called on blob with none null data for pool '%s'.", owning_pool->m_poolName);

    m_owningPool = owning_pool;
    m_totalBlocksInBlob = count;
    m_usedBlocksInBlob = 0;

    int alloc_size = Round_Up_Word_Size(owning_pool->m_allocationSize) + sizeof(MemoryPoolSingleBlock);
    m_blockData = static_cast<char *>(Raw_Allocate_No_Zero(alloc_size * m_totalBlocksInBlob));
    char *current_block = m_blockData;

    for (int i = m_totalBlocksInBlob - 1; i >= 0; --i) {
        MemoryPoolSingleBlock *block_header = reinterpret_cast<MemoryPoolSingleBlock *>(current_block);
        block_header->Init_Block(0, this, owning_pool->m_factory);

        // Move to next block and get poointer to next header. Mask ensures its null on last iteration.
        if (i > 0) {
            current_block = current_block + alloc_size;
            MemoryPoolSingleBlock *next = reinterpret_cast<MemoryPoolSingleBlock *>(current_block);
            block_header->m_nextBlock = next;
            next->m_prevBlock = block_header;
        } else {
            block_header->m_nextBlock = nullptr;
        }
    }

    m_firstFreeBlock = reinterpret_cast<MemoryPoolSingleBlock *>(m_blockData);
}

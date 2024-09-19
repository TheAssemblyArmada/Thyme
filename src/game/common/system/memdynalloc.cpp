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
#include "memdynalloc.h"
#include "critsection.h"
#include "gamememoryinit.h"
#include "memblob.h"
#include "memblock.h"
#include "mempool.h"
#include "mempoolfact.h"
#include <cstring>

using std::memset;

#ifndef GAME_DLL
SimpleCriticalSectionClass *g_dmaCriticalSection = nullptr;
DynamicMemoryAllocator *g_dynamicMemoryAllocator = nullptr;
#endif

DynamicMemoryAllocator::DynamicMemoryAllocator() :
    m_factory(nullptr), m_nextDmaInFactory(nullptr), m_poolCount(0), m_usedBlocksInDma(0), m_rawBlocks(0)
{
    memset(m_pools, 0, sizeof(m_pools));
}

void DynamicMemoryAllocator::Init(MemoryPoolFactory *factory, int subpools, PoolInitRec const *const params)
{
    PoolInitRec const defaults[7] = {
        { "dmaPool_16", 16, 64, 64 },
        { "dmaPool_32", 32, 64, 64 },
        { "dmaPool_64", 64, 64, 64 },
        { "dmaPool_128", 128, 64, 64 },
        { "dmaPool_256", 256, 64, 64 },
        { "dmaPool_512", 512, 64, 64 },
        { "dmaPool_1024", 1024, 64, 64 },
    };

    PoolInitRec const *init_list = params;
    m_poolCount = subpools;

    // If we didn't get passed any initialisation or no count of sub pools, use the defaults
    if (m_poolCount <= 0 || init_list == nullptr) {
        m_poolCount = 7;
        init_list = defaults;
    }

    m_factory = factory;
    m_usedBlocksInDma = 0;

    if (m_poolCount > 8) {
        m_poolCount = 8;
    }

    for (int i = 0; i < m_poolCount; ++i) {
        m_pools[i] = m_factory->Create_Memory_Pool(&init_list[i]);
    }
}

DynamicMemoryAllocator::~DynamicMemoryAllocator()
{
    captainslog_dbgassert(m_usedBlocksInDma == 0, "Destroying none empty DMA.");

    for (int i = 0; i < m_poolCount; ++i) {
        m_factory->Destroy_Memory_Pool(m_pools[i]);
        m_pools[i] = nullptr;
    }

    for (MemoryPoolSingleBlock *b = m_rawBlocks; b != nullptr; b = m_rawBlocks) {
        Free_Bytes(b->Get_User_Data());
    }
}

MemoryPool *DynamicMemoryAllocator::Find_Pool_For_Size(int size)
{
    if (m_poolCount <= 0) {
        return nullptr;
    }

    for (int i = 0; i < m_poolCount; ++i) {
        if (size <= m_pools[i]->m_allocationSize) {
            return m_pools[i];
        }
    }

    return nullptr;
}

void DynamicMemoryAllocator::Add_To_List(DynamicMemoryAllocator **head)
{
    m_nextDmaInFactory = *head;
    *head = this;
}

void DynamicMemoryAllocator::Remove_From_List(DynamicMemoryAllocator **head)
{
    DynamicMemoryAllocator *prev_dma = nullptr;
    DynamicMemoryAllocator *dma = *head;

    if (*head == nullptr) {
        return;
    }

    while (dma != this) {
        prev_dma = dma;
        dma = dma->m_nextDmaInFactory;

        if (dma == nullptr) {
            return;
        }
    }

    if (prev_dma != nullptr) {
        prev_dma->m_nextDmaInFactory = m_nextDmaInFactory;
    } else {
        *head = m_nextDmaInFactory;
    }
}

void *DynamicMemoryAllocator::Allocate_Bytes_No_Zero(int bytes)
{
#ifdef __SANITIZE_ADDRESS__
    return malloc(bytes);
#else
    ScopedCriticalSectionClass cs(g_dmaCriticalSection);

    MemoryPool *mp = Find_Pool_For_Size(bytes);
    void *block;

    if (mp != nullptr) {
        block = mp->Allocate_Block_No_Zero();
    } else {
        block = MemoryPoolSingleBlock::Raw_Allocate_Single_Block(&m_rawBlocks, bytes, m_factory)->Get_User_Data();
    }

    ++m_usedBlocksInDma;

    return block;
#endif
}

void *DynamicMemoryAllocator::Allocate_Bytes(int bytes)
{
    void *block = Allocate_Bytes_No_Zero(bytes);
    memset(block, 0, bytes);

    return block;
}

void DynamicMemoryAllocator::Free_Bytes(void *block)
{
    if (block == nullptr) {
        return;
    }
#ifdef __SANITIZE_ADDRESS__
    free(block);
#else
    ScopedCriticalSectionClass cs(g_dmaCriticalSection);

    MemoryPoolSingleBlock *sblock = MemoryPoolSingleBlock::Recover_Block_From_User_Data(block);

    if (sblock->m_owningBlob != nullptr) {
        sblock->m_owningBlob->m_owningPool->Free_Block(block);
    } else {
        sblock->Remove_Block_From_List(&m_rawBlocks);
        Raw_Free(sblock);
    }

    --m_usedBlocksInDma;
#endif
}

int DynamicMemoryAllocator::Get_Actual_Allocation_Size(int bytes)
{
#ifndef __SANITIZE_ADDRESS__
    MemoryPool *mp = Find_Pool_For_Size(bytes);

    if (mp != nullptr) {
        return mp->m_allocationSize;
    }
#endif
    return bytes;
}

void DynamicMemoryAllocator::Reset()
{
    for (int i = 0; i < m_poolCount; ++i) {
        if (m_pools[i] != nullptr) {
            m_pools[i]->Reset();
        }
    }

    for (MemoryPoolSingleBlock *sb = m_rawBlocks; sb != nullptr; sb = m_rawBlocks) {
        Free_Bytes(sb->Get_User_Data());
    }

    m_usedBlocksInDma = 0;
}

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
#include "mempool.h"
#include "critsection.h"
#include "memblob.h"
#include "memblock.h"
#include <algorithm>
#include <cstring>

using std::memset;

#ifndef GAME_DLL
SimpleCriticalSectionClass *g_memoryPoolCriticalSection = nullptr;
#endif

MemoryPool::MemoryPool() :
    m_factory(nullptr),
    m_nextPoolInFactory(nullptr),
    m_poolName(""),
    m_allocationSize(0),
    m_initialAllocationCount(0),
    m_overflowAllocationCount(0),
    m_usedBlocksInPool(0),
    m_totalBlocksInPool(0),
    m_peakUsedBlocksInPool(0),
    m_firstBlob(nullptr),
    m_lastBlob(nullptr),
    m_firstBlobWithFreeBlocks(nullptr)
{
}

MemoryPool::~MemoryPool()
{
    for (MemoryPoolBlob *b = m_firstBlob; b != nullptr; b = m_firstBlob) {
        Free_Blob(b);
    }
}

void MemoryPool::Init(MemoryPoolFactory *factory, const char *name, int size, int count, int overflow)
{
    m_factory = factory;
    m_poolName = name;
    m_allocationSize = Round_Up_Word_Size(size);
    m_overflowAllocationCount = overflow;
    m_initialAllocationCount = count;
    m_usedBlocksInPool = 0;
    m_totalBlocksInPool = 0;
    m_peakUsedBlocksInPool = 0;
    m_firstBlob = nullptr;
    m_lastBlob = nullptr;
    m_firstBlobWithFreeBlocks = nullptr;
    Create_Blob(count);
}

MemoryPoolBlob *MemoryPool::Create_Blob(int count)
{
    MemoryPoolBlob *blob = new MemoryPoolBlob;
    blob->Init_Blob(this, count);
    blob->Add_Blob_To_List(&m_firstBlob, &m_lastBlob);

    captainslog_dbgassert(m_firstBlobWithFreeBlocks == nullptr, "Expected nullptr here");

    m_firstBlobWithFreeBlocks = blob;
    m_totalBlocksInPool += count;

    return blob;
}

int MemoryPool::Free_Blob(MemoryPoolBlob *blob)
{
    captainslog_dbgassert(blob->m_owningPool == this, "Blob does not belong to this pool");

    blob->Remove_Blob_From_List(&m_firstBlob, &m_lastBlob);

    if (m_firstBlobWithFreeBlocks == blob) {
        m_firstBlobWithFreeBlocks = m_firstBlob;
    }

    int blob_alloc = blob->m_totalBlocksInBlob * m_allocationSize + sizeof(*blob);
    m_usedBlocksInPool -= blob->m_usedBlocksInBlob;
    m_totalBlocksInPool -= blob->m_totalBlocksInBlob;

    delete blob;

    return blob_alloc;
}

void *MemoryPool::Allocate_Block_No_Zero()
{
#ifdef __SANITIZE_ADDRESS__
    return malloc(m_allocationSize);
#else
    ScopedCriticalSectionClass scs(g_memoryPoolCriticalSection);

    if (m_firstBlobWithFreeBlocks != nullptr && m_firstBlobWithFreeBlocks->m_firstFreeBlock == nullptr) {
        MemoryPoolBlob *i;
        for (i = m_firstBlob; i != nullptr; i = i->m_nextBlob) {
            if (i->m_firstFreeBlock != nullptr) {
                break;
            }
        }

        m_firstBlobWithFreeBlocks = i;
    }

    if (m_firstBlobWithFreeBlocks == nullptr) {
        captainslog_relassert(m_overflowAllocationCount != 0,
            0xDEAD0002,
            "Attempting to allocate overflow blocks when m_overflowAllocationCount is 0.");
        Create_Blob(m_overflowAllocationCount);
    }

    MemoryPoolSingleBlock *block = m_firstBlobWithFreeBlocks->Allocate_Single_Block();
    ++m_usedBlocksInPool;
    m_peakUsedBlocksInPool = std::max(m_peakUsedBlocksInPool, m_usedBlocksInPool);

    return block->Get_User_Data();
#endif
}

void *MemoryPool::Allocate_Block()
{
    void *block = Allocate_Block_No_Zero();
    memset(block, 0, m_allocationSize);

    return block;
}

void MemoryPool::Free_Block(void *block)
{
    if (block == nullptr) {
        return;
    }
#ifdef __SANITIZE_ADDRESS__
    free(block);
#else
    ScopedCriticalSectionClass scs(g_memoryPoolCriticalSection);
    MemoryPoolSingleBlock *mp_block = MemoryPoolSingleBlock::Recover_Block_From_User_Data(block);
    MemoryPoolBlob *mp_blob = mp_block->m_owningBlob;

    captainslog_dbgassert(mp_blob != nullptr && mp_blob->m_owningPool == this, "Block is not part of this pool");

    mp_blob->Free_Single_Block(mp_block);

    if (m_firstBlobWithFreeBlocks == nullptr) {
        m_firstBlobWithFreeBlocks = mp_blob;
        --m_usedBlocksInPool;
    }
#endif
}

int MemoryPool::Count_Blobs()
{
    int count = 0;

    for (MemoryPoolBlob *i = m_firstBlob; i != nullptr; i = i->m_nextBlob) {
        ++count;
    }

    return count;
}

int MemoryPool::Release_Empties()
{
    ScopedCriticalSectionClass scs(g_memoryPoolCriticalSection);

    int count = 0;

    for (MemoryPoolBlob *i = m_firstBlob; i != nullptr; i = i->m_nextBlob) {
        if (i->m_usedBlocksInBlob == 0) {
            count += Free_Blob(i);
        }
    }

    return count;
}

void MemoryPool::Reset()
{
    ScopedCriticalSectionClass scs(g_memoryPoolCriticalSection);

    for (MemoryPoolBlob *i = m_firstBlob; i != nullptr; i = m_firstBlob) {
        Free_Blob(i);
    }

    m_firstBlob = nullptr;
    m_lastBlob = nullptr;
    m_firstBlob = nullptr;

    Init(m_factory, m_poolName, m_allocationSize, m_initialAllocationCount, m_overflowAllocationCount);
}

void MemoryPool::Add_To_List(MemoryPool **head)
{
    m_nextPoolInFactory = *head;
    *head = this;
}

void MemoryPool::Remove_From_List(MemoryPool **head)
{
    if (*head == nullptr) {
        return;
    }

    MemoryPool *check = *head;
    MemoryPool *last = nullptr;

    while (check != this) {
        last = check;
        check = check->m_nextPoolInFactory;

        if (check == nullptr) {
            return;
        }
    }

    if (last != nullptr) {
        last->m_nextPoolInFactory = m_nextPoolInFactory;
    } else {
        *head = m_nextPoolInFactory;
    }
}

////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEMEMORY.CPP
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
#include "gamememory.h"
#include "gamememoryinit.h"
#include "critsection.h"
#include "gamedebug.h"
#include "minmax.h"

//////////
// Globals
//////////
SimpleCriticalSectionClass* MemoryPoolCriticalSection = nullptr;
SimpleCriticalSectionClass* DmaCriticalSection = nullptr;

bool ThePreMainInitFlag = false;
bool TheMainInitFlag = false;

////////////////////////
// MemoryPoolSingleBlock
////////////////////////

void MemoryPoolSingleBlock::Init_Block(int size, MemoryPoolBlob *owning_blob, MemoryPoolFactory *owning_fact)
{
    //assert(owning_fact != nullptr);
    NextBlock = 0;
    PrevBlock = 0;
    OwningBlob = owning_blob;
}

void MemoryPoolSingleBlock::Remove_Block_From_List(MemoryPoolSingleBlock **list_head)
{
    ASSERT_PRINT(OwningBlob == nullptr, "This function should only be used on raw blocks");

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

void MemoryPoolSingleBlock::Add_Block_To_List(MemoryPoolSingleBlock **list_head)
{
    NextBlock = *list_head;

    if ( *list_head != nullptr ) {
        (*list_head)->PrevBlock = this;
    }

    *list_head = this;
}

MemoryPoolSingleBlock *MemoryPoolSingleBlock::Recover_Block_From_User_Data(void *data)
{
    if ( data != nullptr ) {
        return reinterpret_cast<MemoryPoolSingleBlock *>(static_cast<char *>(data) - sizeof(MemoryPoolSingleBlock));
    } else {
        ASSERT_PRINT(false, "null data");

        return nullptr;
    }
}

MemoryPoolSingleBlock *MemoryPoolSingleBlock::Raw_Allocate_Single_Block(MemoryPoolSingleBlock **list_head, int size, MemoryPoolFactory *owning_fact)
{
    MemoryPoolSingleBlock *block = static_cast<MemoryPoolSingleBlock*>(Raw_Allocate_No_Zero(Round_Up_Word_Size(size) + sizeof(MemoryPoolSingleBlock)));
    block->Init_Block(size, nullptr, owning_fact);
    block->Add_Block_To_List(list_head);

    return block;
}

/////////////////
// MemoryPoolBlob
/////////////////

MemoryPoolBlob::MemoryPoolBlob() :
    OwningPool(nullptr),
    NextBlob(nullptr),
    PrevBlob(nullptr),
    FirstFreeBlock(nullptr),
    UsedBlocksInBlob(0),
    TotalBlocksInBlob(0),
    BlockData(nullptr)
{

}

MemoryPoolBlob::~MemoryPoolBlob()
{
    Raw_Free(BlockData);
}

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
        block_header->Init_Block(0, this, nullptr);

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

void MemoryPoolBlob::Add_Blob_To_List(MemoryPoolBlob **head, MemoryPoolBlob **tail)
{
    NextBlob = 0;
    PrevBlob = *tail;

    if ( *tail != nullptr ) {
        (*tail)->NextBlob = this;
    }

    if ( *head == nullptr ) {
        *head = this;
    }

    *tail = this;
}

void MemoryPoolBlob::Remove_Blob_From_List(MemoryPoolBlob **head, MemoryPoolBlob **tail)
{
    if ( *head == this ) {
        *head = NextBlob;
    } else {
        PrevBlob->NextBlob = NextBlob;
    }

    if ( *tail == this ) {
        *tail = PrevBlob;
    } else {
        NextBlob->PrevBlob = PrevBlob;
    }
}

MemoryPoolSingleBlock *MemoryPoolBlob::Allocate_Single_Block()
{
    //ASSERT_PRINT(UsedBlocksInBlob < TotalBlocksInBlob, "Trying to allocate when all blocks allocated in blob for pool %s\n", OwningPool->PoolName);
    //ASSERT_PRINT(FirstFreeBlock != nullptr, "Trying to allocated block from blob with null FirstFreeBlock for pool %s\n", OwningPool->PoolName);
    MemoryPoolSingleBlock *block = FirstFreeBlock;
    FirstFreeBlock = block->NextBlock;
    ++UsedBlocksInBlob;

    return block;
}

void MemoryPoolBlob::Free_Single_Block(MemoryPoolSingleBlock *block)
{
    block->Add_Block_To_List(&FirstFreeBlock);
    //MemoryPoolSingleBlock *next_block = FirstFreeBlock;
    //block->NextBlock = next_block;

    //if ( next_block != nullptr ) {
    //    next_block->PrevBlock = block;
    //}

    --UsedBlocksInBlob;
    //FirstFreeBlock = block;
}

/////////////
// MemoryPool
/////////////

MemoryPool::MemoryPool() :
    Factory(nullptr),
    NextPoolInFactory(nullptr),
    PoolName(""),
    AllocationSize(0),
    InitialAllocationCount(0),
    OverflowAllocationCount(0),
    UsedBlocksInPool(0),
    TotalBlocksInPool(0),
    PeakUsedBlocksInPool(0),
    FirstBlob(nullptr),
    LastBlob(nullptr),
    FirstBlobWithFreeBlocks(nullptr)
{

}

MemoryPool::~MemoryPool()
{
    for ( MemoryPoolBlob *b = FirstBlob; b != nullptr; b = FirstBlob ) {
        Free_Blob(b);
    }
}

void MemoryPool::Init(MemoryPoolFactory *factory, char const *name, int size, int count, int overflow)
{
    Factory = factory;
    PoolName = name;
    AllocationSize = Round_Up_Word_Size(size);
    OverflowAllocationCount = overflow;
    InitialAllocationCount = count;
    UsedBlocksInPool = 0;
    TotalBlocksInPool = 0;
    PeakUsedBlocksInPool = 0;
    FirstBlob = nullptr;
    LastBlob = nullptr;
    FirstBlobWithFreeBlocks = nullptr;
    Create_Blob(count);
}

MemoryPoolBlob *MemoryPool::Create_Blob(int count)
{
    MemoryPoolBlob *blob = new MemoryPoolBlob;
    blob->Init_Blob(this, count);
    blob->Add_Blob_To_List(&FirstBlob, &LastBlob);

    ASSERT_PRINT(FirstBlobWithFreeBlocks == nullptr, "Expected nullptr here");

    FirstBlobWithFreeBlocks = blob;
    TotalBlocksInPool += count;

    return blob;
}

int MemoryPool::Free_Blob(MemoryPoolBlob *blob)
{
    ASSERT_PRINT(blob->OwningPool == this, "Blob does not belong to this pool");

    blob->Remove_Blob_From_List(&FirstBlob, &LastBlob);

    if ( FirstBlobWithFreeBlocks == blob ) {
        FirstBlobWithFreeBlocks = FirstBlob;
    }

    int blob_alloc = blob->TotalBlocksInBlob * AllocationSize + sizeof(*blob);
    UsedBlocksInPool -= blob->UsedBlocksInBlob;
    TotalBlocksInPool -= blob->TotalBlocksInBlob;

    delete blob;

    return blob_alloc;
}

void *MemoryPool::Allocate_Block_No_Zero()
{
    ScopedCriticalSectionClass scs(MemoryPoolCriticalSection);

    if ( FirstBlobWithFreeBlocks != nullptr && FirstBlobWithFreeBlocks->FirstFreeBlock == nullptr ) {
        MemoryPoolBlob *i;
        for ( i = FirstBlob; i != nullptr; i = i->NextBlob ) {
            if ( i->FirstFreeBlock != nullptr ) {
                break;
            }
        }

        FirstBlobWithFreeBlocks = i;
    }

    if ( FirstBlobWithFreeBlocks == nullptr ) {
        ASSERT_THROW(OverflowAllocationCount != 0, std::bad_alloc());
        Create_Blob(OverflowAllocationCount);
    }

    MemoryPoolSingleBlock *block = FirstBlobWithFreeBlocks->Allocate_Single_Block();
    ++UsedBlocksInPool;

    //TODO convert to MAX()
    //if ( PeakUsedBlocksInPool < UsedBlocksInPool ) {
    //    PeakUsedBlocksInPool = UsedBlocksInPool;
    //}

    PeakUsedBlocksInPool = MAX(PeakUsedBlocksInPool, UsedBlocksInPool);

    return block->Get_User_Data();
}

void *MemoryPool::Allocate_Block()
{
    void *block = Allocate_Block_No_Zero();
    memset(block, 0, AllocationSize);

    return block;
}

void MemoryPool::Free_Block(void *block)
{
    if ( block == nullptr ) {
        return;
    }

    ScopedCriticalSectionClass scs(MemoryPoolCriticalSection);
    MemoryPoolSingleBlock *mp_block = MemoryPoolSingleBlock::Recover_Block_From_User_Data(block);
    MemoryPoolBlob *mp_blob = mp_block->OwningBlob;

    ASSERT_PRINT(mp_blob != nullptr && mp_blob->OwningPool == this, "Block is not part of this pool");

    mp_blob->Free_Single_Block(mp_block);

    if ( FirstBlobWithFreeBlocks == nullptr ) {
        FirstBlobWithFreeBlocks = mp_blob;
        --UsedBlocksInPool;
    }
}

int MemoryPool::Count_Blobs()
{
    int count = 0;

    for ( MemoryPoolBlob *i = FirstBlob; i != nullptr; i = i->NextBlob ) {
        ++count;
    }

    return count;
}

int MemoryPool::Release_Empties()
{
    int count = 0;

    for ( MemoryPoolBlob *i = FirstBlob; i != nullptr; i = i->NextBlob ) {
        if ( i->UsedBlocksInBlob == 0 ) {
            count += Free_Blob(i);
        }
    }

    return  count;
}

void MemoryPool::Reset()
{
    for ( MemoryPoolBlob *i = FirstBlob; i != nullptr; i = FirstBlob ) {
        Free_Blob(i);
    }

    FirstBlob = nullptr;
    LastBlob = nullptr;
    FirstBlob = nullptr;

    Init(Factory, PoolName, AllocationSize, InitialAllocationCount, OverflowAllocationCount);
}

void MemoryPool::Add_To_List(MemoryPool **head)
{
    NextPoolInFactory = *head;
    *head = this;
}

void MemoryPool::Remove_From_List(MemoryPool **head)
{
    if ( *head == nullptr ) {
        return;
    }

    MemoryPool *check = *head;
    MemoryPool *last = nullptr;

    while ( check != this ) {
        last = check;
        check = check->NextPoolInFactory;

        if ( check == nullptr ) {
            return;
        }
    }

    if ( last != nullptr ) {
        last->NextPoolInFactory = NextPoolInFactory;
    } else {
        *head = NextPoolInFactory;
    }
}

////////////////////
// MemoryPoolFactory
////////////////////

MemoryPoolFactory::~MemoryPoolFactory()
{
    for ( MemoryPool *mp = FirstPoolInFactory; FirstPoolInFactory != nullptr; mp = FirstPoolInFactory ) {
        Destroy_Memory_Pool(mp);
    }

    for ( DynamicMemoryAllocator *dma = FirstDmaInFactory; FirstDmaInFactory != nullptr; dma = FirstDmaInFactory ) {
        Destroy_Dynamic_Memory_Allocator(dma);
    }
}

MemoryPool *MemoryPoolFactory::Create_Memory_Pool(PoolInitRec const *params)
{
    return Create_Memory_Pool(
        params->PoolName,
        params->AllocationSize,
        params->InitialAllocationCount,
        params->OverflowAllocationCount
    );
}

MemoryPool *MemoryPoolFactory::Create_Memory_Pool(char const *name, int size, int count, int overflow)
{
    MemoryPool *pool = Find_Memory_Pool(name);

    if ( pool != nullptr) {
        ASSERT_PRINT(pool->AllocationSize == size, "Pool size mismatch");

        return pool;
    }

    User_Memory_Adjust_Pool_Size(name, count, overflow);

    //
    // Count and overflow should never end up as 0 from adjustment.
    //
    ASSERT_THROW(count > 0 && overflow > 0, std::bad_alloc());
    
    pool = new MemoryPool;
    pool->Init(this, name, size, count, overflow);
    pool->Add_To_List(&FirstPoolInFactory);

    return pool;
}

void MemoryPoolFactory::Destroy_Memory_Pool(MemoryPool *pool)
{
    //
    // Can't destroy a none existent pool.
    //
    if ( pool == nullptr ) {
        return;
    }

    ASSERT_PRINT(pool->UsedBlocksInPool == 0, "Destroying none empty pool.");

    pool->Remove_From_List(&FirstPoolInFactory);
    delete pool;
}

MemoryPool *MemoryPoolFactory::Find_Memory_Pool(char const *name)
{
    MemoryPool *pool = nullptr;

    //
    // Go through the pools and break on matching requested name.
    //
    for ( pool = FirstPoolInFactory; pool != nullptr; pool = pool->NextPoolInFactory ) {
        if ( strcmp(pool->PoolName, name) == 0 ) {
            break;
        }
    }

    return pool;
}

DynamicMemoryAllocator *MemoryPoolFactory::Create_Dynamic_Memory_Allocator(int subpools, PoolInitRec const *const params)
{
    DynamicMemoryAllocator *allocator = new DynamicMemoryAllocator;
    allocator->Init(this, subpools, params);
    allocator->Add_To_List(&FirstDmaInFactory);

    return allocator;
}

void MemoryPoolFactory::Destroy_Dynamic_Memory_Allocator(DynamicMemoryAllocator *allocator)
{
    if ( allocator == nullptr ) {
        return;
    }

    allocator->Remove_From_List(&FirstDmaInFactory);
    delete allocator;
}

void MemoryPoolFactory::Reset()
{
    for ( MemoryPool *mp = FirstPoolInFactory; mp != nullptr; mp = mp->NextPoolInFactory ) {
        mp->Reset();
    }

    for ( DynamicMemoryAllocator *dma = FirstDmaInFactory; dma != nullptr; dma = dma->NextDmaInFactory ) {
        dma->Reset();
    }
}

/////////////////////////
// DynamicMemoryAllocator
/////////////////////////

DynamicMemoryAllocator::DynamicMemoryAllocator() :
    Factory(nullptr),
    NextDmaInFactory(nullptr),
    PoolCount(0),
    UsedBlocksInDma(0),
    RawBlocks(0)
{
    memset(Pools, 0, sizeof(Pools));
}

void DynamicMemoryAllocator::Init(MemoryPoolFactory *factory, int subpools, PoolInitRec const *const params)
{
    PoolInitRec const defaults[7] = {
        { "dmaPool_16",   16,   64, 64 },
        { "dmaPool_32",   32,   64, 64 },
        { "dmaPool_64",   64,   64, 64 },
        { "dmaPool_128",  128,  64, 64 },
        { "dmaPool_256",  256,  64, 64 },
        { "dmaPool_512",  512,  64, 64 },
        { "dmaPool_1024", 1024, 64, 64 },
    };

    PoolInitRec const *init_list = params;
    PoolCount = subpools;

    //
    // If we didn't get passed any initialisation or no count of sub pools, use the defaults
    //
    if ( PoolCount <= 0 || init_list == nullptr ) {
        PoolCount = 7;
        init_list = defaults;
    }

    Factory = factory;
    UsedBlocksInDma = 0;

    if ( PoolCount > 8 ) {
        PoolCount = 8;
    }

    for ( int i = 0; i < PoolCount; ++i ) {
        Pools[i] = Factory->Create_Memory_Pool(&init_list[i]);
    }
}

DynamicMemoryAllocator::~DynamicMemoryAllocator()
{
    ASSERT_PRINT(UsedBlocksInDma, "Destroying none empty DMA.");

    for ( int i = 0; i < PoolCount; ++i ) {
        Factory->Destroy_Memory_Pool(Pools[i]);
        Pools[i] = nullptr;
    }

    for ( MemoryPoolSingleBlock *b = RawBlocks; b != nullptr; b = RawBlocks ) {
        Free_Bytes(b->Get_User_Data());
    }
}

MemoryPool *DynamicMemoryAllocator::Find_Pool_For_Size(int size)
{
    if ( PoolCount <= 0 ) {
        return nullptr;
    }

    for ( int i = 0; i < PoolCount; ++i ) {
        if ( size <= Pools[i]->AllocationSize ) {
            return Pools[i];
        }
    }

    return nullptr;
}

void DynamicMemoryAllocator::Add_To_List(DynamicMemoryAllocator **head)
{
    NextDmaInFactory = *head;
    *head = this;
}

void DynamicMemoryAllocator::Remove_From_List(DynamicMemoryAllocator **head)
{
    DynamicMemoryAllocator *prev_dma = nullptr;
    DynamicMemoryAllocator *dma = *head;
    
    if ( *head == nullptr ) {
        return;
    }

    while ( dma != this ) {
        prev_dma = dma;
        dma = dma->NextDmaInFactory;

        if ( dma == nullptr ) {
            return;
        }
    }

    if ( prev_dma != nullptr ) {
        prev_dma->NextDmaInFactory = NextDmaInFactory;
    } else {
        *head = NextDmaInFactory;
    }
}

void *DynamicMemoryAllocator::Allocate_Bytes_No_Zero(int bytes)
{
    ScopedCriticalSectionClass cs(DmaCriticalSection);

    MemoryPool *mp = Find_Pool_For_Size(bytes);
    void *block;

    if ( mp != nullptr ) {
        block = mp->Allocate_Block_No_Zero();
    } else {
        block = MemoryPoolSingleBlock::Raw_Allocate_Single_Block(&RawBlocks, bytes, Factory)->Get_User_Data();
    }

    ++UsedBlocksInDma;

    return block;
}

void *DynamicMemoryAllocator::Allocate_Bytes(int bytes)
{
    void *block = Allocate_Bytes_No_Zero(bytes);
    memset(block, 0, bytes);

    return block;
}

void DynamicMemoryAllocator::Free_Bytes(void *block)
{
    if ( block == nullptr ) {
        return;
    }

    ScopedCriticalSectionClass cs(DmaCriticalSection);

    MemoryPoolSingleBlock *sblock = MemoryPoolSingleBlock::Recover_Block_From_User_Data(block);

    if ( sblock->OwningBlob != nullptr ) {
        sblock->OwningBlob->OwningPool->Free_Block(block);
    } else {
        sblock->Remove_Block_From_List(&RawBlocks);
        Raw_Free(sblock);
    }

    --UsedBlocksInDma;
}

int DynamicMemoryAllocator::Get_Actual_Allocation_Size(int bytes)
{
    MemoryPool *mp = Find_Pool_For_Size(bytes);

    if ( mp != nullptr ) {
        return mp->AllocationSize;
    }

    return bytes;
}

void DynamicMemoryAllocator::Reset()
{
    for ( int i = 0; i < PoolCount; ++i ) {
        if ( Pools[i] != nullptr ) {
            Pools[i]->Reset();
        }
    }

    for ( MemoryPoolSingleBlock *sb = RawBlocks; sb != nullptr; sb = RawBlocks ) {
        Free_Bytes(sb->Get_User_Data());
    }

    UsedBlocksInDma = 0;
}

///////////////////////////////////
// Memory Manager Control Functions
///////////////////////////////////

void Init_Memory_Manager()
{
    int param_count;
    PoolInitRec const *params;

    if ( TheMemoryPoolFactory == nullptr ) {
        DEBUG_LOG("Memory Manager initialising normally.\n");
        User_Memory_Get_DMA_Params(&param_count, &params);
        TheMemoryPoolFactory = new MemoryPoolFactory;
        TheMemoryPoolFactory->Init();
        TheDynamicMemoryAllocator = TheMemoryPoolFactory->Create_Dynamic_Memory_Allocator(param_count, params);
        User_Memory_Init_Pools();
        ThePreMainInitFlag = false;
    }

    //
    // Check that new and delete both use our custom implementation.
    // 
    TheLinkChecker = 0;

    DEBUG_LOG("Checking memory manager operators are linked, link checker at %d\n", TheLinkChecker);

    char *tmp = new char;
    delete tmp;
    tmp = new char[8];
    delete[] tmp;
    SimpleCriticalSectionClass *tmp2 = new SimpleCriticalSectionClass;
    delete tmp2;

    if ( TheLinkChecker != 6 ) {
        DEBUG_LOG("Not linked correct new and delete operators, checker has value %d\n", TheLinkChecker);
        exit(-1);
    }

    DEBUG_LOG("Memory manager operators passed check, link checker at %d\n", TheLinkChecker);

    TheMainInitFlag = true;
}

void Init_Memory_Manager_Pre_Main()
{
    int param_count;
    PoolInitRec const *params;

    if ( TheMemoryPoolFactory == nullptr ) {
        DEBUG_INIT(DEBUG_LOG_TO_FILE);
        DEBUG_LOG("Memory Manager initialising prior to WinMain\n");

        User_Memory_Get_DMA_Params(&param_count, &params);
        TheMemoryPoolFactory = new MemoryPoolFactory;
        TheMemoryPoolFactory->Init();
        TheDynamicMemoryAllocator = TheMemoryPoolFactory->Create_Dynamic_Memory_Allocator(param_count, params);
        User_Memory_Init_Pools();
        ThePreMainInitFlag = true;
    }
}

void Shutdown_Memory_Manager()
{
    if ( !ThePreMainInitFlag ) {
        if ( TheMemoryPoolFactory != nullptr ) {
            if ( TheDynamicMemoryAllocator != nullptr ) {
                TheMemoryPoolFactory->Destroy_Dynamic_Memory_Allocator(TheDynamicMemoryAllocator);
                TheDynamicMemoryAllocator = nullptr;
            }

            delete TheMemoryPoolFactory;
            TheMemoryPoolFactory = nullptr;
        }
    }

    TheMainInitFlag = false;
}

////////////////////////
// Replacement operators
////////////////////////

MemoryPool *Create_Named_Pool(char const *name, int size)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();
    return TheMemoryPoolFactory->Create_Memory_Pool(name, size, 0, 0);
}

//
// These all override the global news and deletes just by being linked.
//
void *New_New(size_t bytes)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();
    return TheDynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void *operator new(size_t bytes)
{
    return New_New(bytes);
}

void *New_Array_New(size_t bytes)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();
    return TheDynamicMemoryAllocator->Allocate_Bytes(bytes);
}

void *operator new[](size_t bytes)
{
    return New_Array_New(bytes);
}

void New_Delete(void *ptr)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();
    TheDynamicMemoryAllocator->Free_Bytes(ptr);
}

void operator delete(void *ptr)
{
    New_Delete(ptr);
}

void New_Array_Delete(void *ptr)
{
    ++TheLinkChecker;
    Init_Memory_Manager_Pre_Main();
    TheDynamicMemoryAllocator->Free_Bytes(ptr);
}

void operator delete[](void *ptr)
{
    New_Array_Delete(ptr);
}

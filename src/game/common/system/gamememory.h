////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEMEMORY.H
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

#ifndef _GAMEMEMORY_H_
#define _GAMEMEMORY_H_

#include "always.h"
#include "gamedebug.h"
#include "hooker.h"         //Remove once all hooks implemented
#include <stdexcept>

void *New_New(size_t bytes);
void *New_Array_New(size_t bytes);
void New_Delete(void *ptr);
void New_Array_Delete(void *ptr);

// Use GlobalAlloc as the raw allocator on windows to avoid CRT issues.
#ifdef PLATFORM_WINDOWS
inline void *Raw_Allocate(int bytes)
{
    void *r = GlobalAlloc(GMEM_ZEROINIT | GMEM_FIXED, bytes);

    //if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    THROW_ASSERT(r != nullptr, std::bad_alloc());

    return r;
}

inline void *Raw_Allocate_No_Zero(int bytes)
{
    void *r = GlobalAlloc(GMEM_FIXED, bytes);

    //if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    THROW_ASSERT(r != nullptr, std::bad_alloc());

    return r;
}

inline void Raw_Free(void *memory) { if (memory != nullptr) GlobalFree(memory); }

// Otherwise use standard allocators.
#else
inline void *Raw_Allocate(int bytes)
{
    void *r = calloc(1, bytes);

    //if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    THROW_ASSERT(r != nullptr, std::bad_alloc());

    return r;
}

inline void *Raw_Allocate_No_Zero(int bytes)
{
    void *r = malloc(bytes);

    //if (r == nullptr) {
    //    throw std::bad_alloc();
    //}

    THROW_ASSERT(r != nullptr, std::bad_alloc());

    return r;
}

inline void Raw_Free(void *memory) { if (memory != nullptr) free(memory); }
#endif

struct PoolInitRec;

class MemoryPoolFactory;
class MemoryPoolBlob;
class MemoryPool;
class DynamicMemoryAllocator;
class SimpleCriticalSectionClass;

// Allocated a critical section in WinMain, hooked to original currently.
extern SimpleCriticalSectionClass* MemoryPoolCriticalSection;
extern SimpleCriticalSectionClass* DmaCriticalSection;

#define TheMemoryPoolFactory (Make_Global<MemoryPoolFactory*>(0x00A29B94))
#define TheDynamicMemoryAllocator (Make_Global<DynamicMemoryAllocator*>(0x00A29B98))
#define TheLinkChecker (Make_Global<int>(0x00A29B9C))
extern bool ThePreMainInitFlag;
extern bool TheMainInitFlag;

void Init_Memory_Manager();
void Init_Memory_Manager_Pre_Main();
void Shutdown_Memory_Manager();
MemoryPool *Create_Named_Pool(char const *name, int size);

class MemoryPoolSingleBlock
{
    public:
        MemoryPoolSingleBlock() : OwningBlob(nullptr), NextBlock(nullptr), PrevBlock(nullptr) {}
        void Init_Block(int size, MemoryPoolBlob *owning_blob, MemoryPoolFactory *owning_fact);
        void Remove_Block_From_List(MemoryPoolSingleBlock **list_head);
        void Add_Block_To_List(MemoryPoolSingleBlock **list_head);
        void *Get_User_Data() { return reinterpret_cast<void *>(&this[1]); }
        static MemoryPoolSingleBlock *Recover_Block_From_User_Data(void *data);
        static MemoryPoolSingleBlock *Raw_Allocate_Single_Block(MemoryPoolSingleBlock **list_head, int size, MemoryPoolFactory *owning_fact);

        friend class MemoryPoolBlob;
        friend class MemoryPool;
        friend class DynamicMemoryAllocator;
    private:
        MemoryPoolBlob *OwningBlob;
        MemoryPoolSingleBlock *NextBlock;
        MemoryPoolSingleBlock *PrevBlock;
};

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
        MemoryPool *OwningPool;
        MemoryPoolBlob *NextBlob;
        MemoryPoolBlob *PrevBlob;
        MemoryPoolSingleBlock *FirstFreeBlock;
        int UsedBlocksInBlob;
        int TotalBlocksInBlob;
        char *BlockData;
};

class MemoryPool
{
    public:
        MemoryPool();
        ~MemoryPool();
        void Init(MemoryPoolFactory *factory, char const *name, int size, int count, int overflow);
        MemoryPoolBlob *Create_Blob(int count);
        int Free_Blob(MemoryPoolBlob *blob);
        void *Allocate_Block_No_Zero();
        void *Allocate_Block();
        void Free_Block(void *block);
        int Count_Blobs();
        int Release_Empties();
        void Reset();
        void Add_To_List(MemoryPool **head);
        void Remove_From_List(MemoryPool **head);

        void *operator new(size_t size) throw()
        {
            return Raw_Allocate(size);
        }

        void operator delete(void *obj)
        {
            Raw_Free(obj);
        }

        friend class MemoryPoolBlob;
        friend class MemoryPoolFactory;
        friend class DynamicMemoryAllocator;
    private:
        MemoryPoolFactory *Factory;
        MemoryPool *NextPoolInFactory;
        char const *PoolName;
        int AllocationSize;
        int InitialAllocationCount;
        int OverflowAllocationCount;
        int UsedBlocksInPool;
        int TotalBlocksInPool;
        int PeakUsedBlocksInPool;
        MemoryPoolBlob *FirstBlob;
        MemoryPoolBlob *LastBlob;
        MemoryPoolBlob *FirstBlobWithFreeBlocks;
};

class MemoryPoolFactory
{
    public:
        MemoryPoolFactory() : FirstPoolInFactory(nullptr), FirstDmaInFactory(nullptr) {}
        ~MemoryPoolFactory();
        void Init() {}
        MemoryPool *Create_Memory_Pool(PoolInitRec const *params);
        MemoryPool *Create_Memory_Pool(char const *name, int size, int count, int overflow);
        MemoryPool *Find_Memory_Pool(char const *name);
        void Destroy_Memory_Pool(MemoryPool *pool);
        DynamicMemoryAllocator *Create_Dynamic_Memory_Allocator(int subpools, PoolInitRec const *const params);
        void Destroy_Dynamic_Memory_Allocator(DynamicMemoryAllocator *allocator);
        void Reset();

        void *operator new(size_t size) throw()
        {
            return Raw_Allocate_No_Zero(size);
        }

        void operator delete(void *obj)
        {
            Raw_Free(obj);
        }
    private:
        MemoryPool *FirstPoolInFactory;
        DynamicMemoryAllocator *FirstDmaInFactory;
};

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

//
// Base class for any object requiring efficient memory handling.
//
class MemoryPoolObject
{
    public:
    virtual ~MemoryPoolObject() {}
    virtual MemoryPool *Get_Object_Pool() = 0;

    void *operator new(unsigned int size) {}
    void operator delete(void *object) {}
};

//
// Not much to go on for these, looks heavily inlined.
//
class MemoryPoolObjectHolder
{
    public:
    MemoryPoolObjectHolder(MemoryPoolObject *obj) : Obj(obj) {}
    ~MemoryPoolObjectHolder()
    {
        if ( Obj != nullptr ) {
            MemoryPool *mp = Obj->Get_Object_Pool();
            Obj->~MemoryPoolObject();
            mp->Free_Block(Obj);
        }
    }
    void Hold(MemoryPoolObject *obj) { Obj = obj; }
    void Release() { Obj = nullptr; }

    private:
    MemoryPoolObject *Obj;
};

#endif // _GAMEMEMORY_H_

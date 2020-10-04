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
#include "rawalloc.h"

struct PoolInitRec;
class MemoryPool;
class DynamicMemoryAllocator;

class MemoryPoolFactory
{
public:
    MemoryPoolFactory() : m_firstPoolInFactory(nullptr), m_firstDmaInFactory(nullptr) {}
    ~MemoryPoolFactory();
    void Init() {}
    MemoryPool *Create_Memory_Pool(PoolInitRec const *params);
    MemoryPool *Create_Memory_Pool(const char *name, int size, int count, int overflow);
    MemoryPool *Find_Memory_Pool(const char *name);
    void Destroy_Memory_Pool(MemoryPool *pool);
    DynamicMemoryAllocator *Create_Dynamic_Memory_Allocator(int subpools, PoolInitRec const *const params);
    void Destroy_Dynamic_Memory_Allocator(DynamicMemoryAllocator *allocator);
    void Reset();

    void *operator new(size_t size) throw() { return Raw_Allocate_No_Zero(size); }

    void operator delete(void *obj) { Raw_Free(obj); }

private:
    MemoryPool *m_firstPoolInFactory;
    DynamicMemoryAllocator *m_firstDmaInFactory;
};

#ifdef GAME_DLL
extern MemoryPoolFactory *&g_memoryPoolFactory;
#else
extern MemoryPoolFactory *g_memoryPoolFactory;
#endif
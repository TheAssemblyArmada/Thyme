/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Classes for reducing heap allocations by allocating large chunks up front.
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
#include "critsection.h"
#include <captainslog.h>

template<typename T, int BLOCK_SIZE> class ObjectPoolClass
{
public:
    ObjectPoolClass() : m_freeListHead(nullptr), m_blockListHead(nullptr), m_freeObjectCount(0), m_totalObjectCount(0) {}

    ~ObjectPoolClass()
    {
        // If you hit the following assert, one or more objects were not freed.
        captainslog_dbgassert(
            m_freeObjectCount == m_totalObjectCount, "Not all memory was returned to the pool before destruction.");

        void *block = m_blockListHead;

        while (block) {
            void *nextBlock = *(void **)block;
            delete[](char *) block;
            block = nextBlock;
        }
    }

    T *Allocate_Object_Memory()
    {
        FastCriticalSectionClass::LockClass lock(m_poolCS);

        // Grow our allocation if we haven't got any free space.
        if (m_freeListHead == nullptr) {
            // Allocate enough space for our block plus space for a pointer to create a linked list.
            void *newBlockListHead = (void *)((void *)new char[sizeof(void *) + sizeof(T[BLOCK_SIZE])]);
            // Pointer vodoo to link the block into the list.
            *(void **)newBlockListHead = m_blockListHead;
            m_blockListHead = newBlockListHead;
            m_freeListHead = (T *)((intptr_t)m_blockListHead + sizeof(void *));

            // Shuffle the free block pointers down the list
            for (int i = 0; i < BLOCK_SIZE; i++) {
                (T *&)m_freeListHead[i] = &m_freeListHead[i + 1];
            }

            // Null the last "next" pointer in the list so we know when we need more.
            (T *&)m_freeListHead[BLOCK_SIZE - 1] = nullptr;
            m_freeObjectCount += BLOCK_SIZE;
            m_totalObjectCount += BLOCK_SIZE;
        }

        // Do an allocation.
        T *object = m_freeListHead;
        m_freeListHead = *(T **)(m_freeListHead);
        m_freeObjectCount--;

        return object;
    }

    void Free_Object_Memory(T *object)
    {
        FastCriticalSectionClass::LockClass lock(m_poolCS);
        *(T **)(object) = m_freeListHead;
        m_freeListHead = object;
        m_freeObjectCount++;
    }

private:
    T *m_freeListHead;
    void *m_blockListHead;
    int m_freeObjectCount;
    int m_totalObjectCount;
    FastCriticalSectionClass m_poolCS;
};

template<typename T, unsigned BLOCKSIZE> class AutoPoolClass
{
public:
    static void *operator new(size_t size)
    {
        captainslog_assert(size == sizeof(T));
        return (void *)m_allocator.Allocate_Object_Memory();
    }

    static void operator delete(void *object)
    {
        if (object != nullptr) {
            m_allocator.Free_Object_Memory((T *)object);
        }
    }

private:
    // Array allocation not supported.
    static void *operator new[](size_t size);
    static void operator delete[](void *object);

// TT Scripts handles this with a singleton class instead, this is easier to hook IMO
#ifdef GAME_DLL
    static ObjectPoolClass<T, BLOCKSIZE> &m_allocator;
#else
    static ObjectPoolClass<T, BLOCKSIZE> m_allocator;
#endif
};

#ifdef GAME_DLL
// Use HOOK_AUTOPOOL macro in the cpp file where an allocator needs to be hooked, use MAKE_AUTOPOOL if it can be normally
// defined.
#define HOOK_AUTOPOOL(name, size, address) \
    ObjectPoolClass<name, size> &AutoPoolClass<name, size>::m_allocator = Make_Global<ObjectPoolClass<name, size>>(address)

#define MAKE_AUTOPOOL(name, size) ObjectPoolClass<name, size> AutoPoolClass<name, size>::m_allocator
#else
// Standalone just provides a templated instantiation so macros don't do anything.
#define HOOK_AUTOPOOL(name, size, address)
#define MAKE_AUTOPOOL(name, size)

template<typename T, unsigned BLOCKSIZE> ObjectPoolClass<T, BLOCKSIZE> AutoPoolClass<T, BLOCKSIZE>::m_allocator;
#endif

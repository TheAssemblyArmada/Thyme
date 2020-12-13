/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for object intended to be allocated from specialised memory pools.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "mempool.h"
#include "mempoolfact.h"
#include <captainslog.h>
#include <new>

//
// Base class for any object requiring efficient memory handling ?
//
class MemoryPoolObject
{
public:
    virtual ~MemoryPoolObject() {}
    virtual MemoryPool *Get_Object_Pool()
    {
        captainslog_error("This should be impossible to call (abstract base class)!");
        return nullptr;
    }

    void operator delete(void *p, void *q) { captainslog_error("This should be impossible to call (abstract base class)!"); }
    void operator delete(void *ptr) { captainslog_error("This should be impossible to call (abstract base class)!"); }

    // Class implementing Get_Object_Pool needs to provide these
    // use macros below to generated them.
};

// Use within a class declaration on a none virtual MemoryPoolObject
// based class to implement required functions. "classname" must match
// the name of the class in which it is used.
#define IMPLEMENT_POOL(classname) \
private: \
    static MemoryPool *Get_Class_Pool() \
    { \
        static MemoryPool *The##classname##Pool; \
        static bool PoolInit; \
        if (!PoolInit) { \
            PoolInit = true; \
            The##classname##Pool = g_memoryPoolFactory->Create_Memory_Pool(#classname, sizeof(classname), -1, -1); \
        } \
        captainslog_dbgassert(The##classname##Pool->Get_Alloc_Size() == sizeof(classname), \
            "Pool %s is wrong size for class (need %d, currently %d)", \
            #classname, \
            sizeof(classname), \
            The##classname##Pool->Get_Alloc_Size()); \
        return The##classname##Pool; \
    } \
\
public: \
    virtual MemoryPool *Get_Object_Pool() override { return Get_Class_Pool(); } \
    void *operator new(size_t size) { return Get_Class_Pool()->Allocate_Block(); } \
    void *operator new(size_t size, void *dst) { return dst; } \
    void operator delete(void *p, void *q) {} \
    void operator delete(void *ptr) { return Get_Class_Pool()->Free_Block(ptr); }

// Use within a class declaration on a none virtual MemoryPoolObject
// based class to implement required functions. "classname" must match
// the name of the class in which it is used, "poolname" should match a
// gamememoryinit.cpp entry.
#define IMPLEMENT_NAMED_POOL(classname, poolname) \
private: \
    static MemoryPool *Get_Class_Pool() \
    { \
        static MemoryPool *The##classname##Pool; \
        static bool PoolInit; \
        if (!PoolInit) { \
            PoolInit = true; \
            The##classname##Pool = g_memoryPoolFactory->Create_Memory_Pool(#poolname, sizeof(classname), -1, -1); \
        } \
        captainslog_dbgassert(The##classname##Pool->Get_Alloc_Size() == sizeof(classname), \
            "Pool %s is wrong size for class (need %d, currently %d)", \
            #classname, \
            sizeof(classname), \
            The##classname##Pool->Get_Alloc_Size()); \
        return The##classname##Pool; \
    } \
\
public: \
    virtual MemoryPool *Get_Object_Pool() override { return Get_Class_Pool(); } \
    void *operator new(size_t size) { return Get_Class_Pool()->Allocate_Block(); } \
    void *operator new(size_t size, void *dst) { return dst; } \
    void operator delete(void *p, void *q) {} \
    void operator delete(void *ptr) { return Get_Class_Pool()->Free_Block(ptr); }

/**
 * @brief Delete an instance of a MemoryPoolObject, ensuring correct pool is used via virtual call.
 */
inline void Delete_Instance(MemoryPoolObject *ptr)
{
    if (ptr != nullptr) {
        MemoryPool *pool = ptr->Get_Object_Pool();
        ptr->~MemoryPoolObject();
        pool->Free_Block(ptr);
    }
}

//
// Class to hold the memory pool for a runtime of a function and if still held destroy it when the function is done.
//
class MemoryPoolObjectHolder
{
public:
    MemoryPoolObjectHolder(MemoryPoolObject *obj = nullptr) : m_obj(obj) {}

    ~MemoryPoolObjectHolder() { Delete_Instance(m_obj); }

    void Hold(MemoryPoolObject *obj)
    {
        if (m_obj != nullptr) {
            captainslog_debug("Already holding!");
        }
        m_obj = obj;
    }

    void Release() { m_obj = nullptr; }

private:
    MemoryPoolObject *m_obj;
};

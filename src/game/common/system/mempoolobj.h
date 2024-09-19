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

#include "always.h"
#include "errorcodes.h"
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
    void Delete_Instance();

protected:
    virtual ~MemoryPoolObject() {}
    virtual MemoryPool *Get_Object_Pool() = 0;

    void *operator new(size_t) = delete;
    void operator delete(void *) { captainslog_dbgassert(0, "This should be impossible to call"); }

    // Class implementing Get_Object_Pool needs to provide these
    // use macros below to generated them.
};

// Use within a class declaration on a none virtual MemoryPoolObject
// based class to implement required functions. "classname" must match
// the name of the class in which it is used, "poolname" should match a
// gamememoryinit.cpp entry.
#define IMPLEMENT_NAMED_POOL(classname, poolname) \
private: \
    static MemoryPool *Get_Class_Pool() \
    { \
        captainslog_dbgassert(g_memoryPoolFactory, "TheMemoryPoolFactory is nullptr"); \
        static MemoryPool *The##classname##Pool = \
            g_memoryPoolFactory->Create_Memory_Pool(#poolname, sizeof(classname), -1, -1); \
        captainslog_dbgassert( \
            The##classname##Pool, "Pool \"%s\" not found (did you set it up in initMemoryPools?)", #poolname); \
        captainslog_dbgassert(The##classname##Pool->Get_Alloc_Size() >= sizeof(classname), \
            "Pool \"%s\" is too small for this class (currently %d, need %d)", \
            #poolname, \
            sizeof(classname), \
            The##classname##Pool->Get_Alloc_Size()); \
        captainslog_dbgassert(The##classname##Pool->Get_Alloc_Size() <= sizeof(classname), \
            "Pool \"%s\" is too large for this class (currently %d, need %d)", \
            #poolname, \
            sizeof(classname), \
            The##classname##Pool->Get_Alloc_Size()); \
        return The##classname##Pool; \
    } \
    virtual MemoryPool *Get_Object_Pool() override { return Get_Class_Pool(); } \
\
public: \
    enum classname##MagicEnum{ classname##_GLUE_NOT_IMPLEMENTED = 0 }; \
\
    void *operator new(size_t size) { return operator new(size, classname##_GLUE_NOT_IMPLEMENTED); } \
    void *operator new(size_t size, classname##MagicEnum) \
    { \
        captainslog_dbgassert(size == sizeof(classname), \
            "The wrong operator new is being called; ensure all objects in the hierarchy have MemoryPoolGlue set up " \
            "correctly"); \
        return Get_Class_Pool()->Allocate_Block(); \
    } \
    void operator delete(void *ptr) { operator delete(ptr, classname##_GLUE_NOT_IMPLEMENTED); } \
    void operator delete(void *ptr, classname##MagicEnum) \
    { \
        captainslog_dbgassert(0, "Please call Delete_Instance instead of delete."); \
        Get_Class_Pool()->Free_Block(ptr); \
    } \
    void *operator new(size_t size, void *where) { return where; } \
    void operator delete(void *ptr, void *where) {}

/* NOTE: Operator delete above must exist because of the pairing operator new. However, it should never be called directly.
 * Reason being, a pointer to a derived class of this class, must use the correct memory pool for deletion. This is possible
 * by calling Get_Object_Pool function. However, operator delete is called after class destructor, and class destructor
 * will destroy virtual table. Therefore operator delete cannot be used.
 */
#define IMPLEMENT_ABSTRACT_POOL(classname) \
protected: \
    virtual MemoryPool *Get_Object_Pool() override { throw CODE_01; } \
    void *operator new(size_t size) = delete; \
    void operator delete(void *ptr) \
    { \
        captainslog_dbgassert(0, "This should be impossible to call (abstract base class)"); \
    } \
\
private:

#define IMPLEMENT_POOL(classname) IMPLEMENT_NAMED_POOL(classname, classname);
// NEW_POOL_OBJ is obsolete. Can be removed.
#define NEW_POOL_OBJ(classname, ...) new classname(__VA_ARGS__)

/**
 * @brief Delete an instance of a MemoryPoolObject, ensuring correct pool is used via virtual call.
 */
inline void MemoryPoolObject::Delete_Instance()
{
    if (this != nullptr) {
        MemoryPool *pool = Get_Object_Pool();
        this->~MemoryPoolObject();
        pool->Free_Block(this);
    }
}

//
// Class to hold the memory pool for a runtime of a function and if still held destroy it when the function is done.
//
class MemoryPoolObjectHolder
{
public:
    MemoryPoolObjectHolder(MemoryPoolObject *obj = nullptr) : m_mpo(obj) {}

    ~MemoryPoolObjectHolder() { m_mpo->Delete_Instance(); }

    void Hold(MemoryPoolObject *obj)
    {
        if (m_mpo != nullptr) {
            captainslog_debug("Already holding!");
        }
        m_mpo = obj;
    }

    void Release() { m_mpo = nullptr; }

private:
    MemoryPoolObject *m_mpo;
};
